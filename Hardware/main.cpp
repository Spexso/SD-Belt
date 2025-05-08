/**
 * main.cpp
 *
 * Main program for controlling a conveyor belt via Arduino.
 * Combines hardware control with an HTTP interface.
 */

#include "ArduinoSerial.h"
#include "httplib.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <sstream>

std::atomic<bool> running(true);

// Signal handler
void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	running = false;
}

class HttpServerHandler
{
public:
	HttpServerHandler(ArduinoSerial *arduinoPtr)
		: arduino(arduinoPtr) {}

		void Init()
		{
			// Setup logger to print every request
			server.set_logger([](const httplib::Request &req, const httplib::Response &res) {
				std::cout << "[HTTP] " << req.method << " " << req.path
						  << " => " << res.status << std::endl;
		
				if (!req.body.empty())
				{
					std::cout << "  Body: " << req.body << std::endl;
				}
			});
		
			// Setup endpoints
			server.Post("/rev", [this](const httplib::Request &, httplib::Response &res)
						{
				std::string response = handleCommand("reverse");
				std::cout << "[/rev] Response: " << response << std::endl;
				res.set_content(response, "text/plain"); });
		
			server.Post("/stop", [this](const httplib::Request &, httplib::Response &res)
						{
				std::string response = handleCommand("stop");
				std::cout << "[/stop] Response: " << response << std::endl;
				res.set_content(response, "text/plain"); });
		}
		

	void Start()
	{
		std::cout << "API SERVER running at http://0.0.0.0:8080\n";
		server.listen_after_bind();
	}

	void Stop()
	{
		server.stop();
	}

	bool Bind()
	{
		return server.bind_to_port("0.0.0.0", 8080);
	}

private:
	ArduinoSerial *arduino;
	httplib::Server server;

	std::string handleCommand(const std::string &cmd)
	{
		if (!arduino->isConnected())
			return "Arduino not connected.";

		std::string serialCommand;

		if (cmd == "start")
			serialCommand = "START\n";
		else if (cmd == "stop")
			serialCommand = "STOP\n";
		else if (cmd == "reverse")
			serialCommand = "REV\n";
		else if (cmd == "status")
			serialCommand = "STATUS\n";
		else if (cmd.rfind("speed=", 0) == 0)
			serialCommand = "PCT:" + cmd.substr(6) + "\n";
		else if (cmd.rfind("servo=", 0) == 0)
			serialCommand = "SERVO:" + cmd.substr(6) + "\n";
		else if (cmd == "dir=1")
			serialCommand = "DIR:1\n";
		else if (cmd == "dir=-1")
			serialCommand = "DIR:-1\n";
		else
			return "ERR: Unknown command";

		std::string response = arduino->sendCommand(serialCommand);
		return response.empty() ? "No response from Arduino." : response;
	}
};

int main(int argc, char *argv[])
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	std::string serialPort = "/dev/ttyUSB0";
	if (argc > 1)
		serialPort = argv[1];

	std::cout << "Starting conveyor belt controller\n";
	std::cout << "Using serial port: " << serialPort << std::endl;

	ArduinoSerial arduino(serialPort);
	if (!arduino.isConnected())
	{
		std::cerr << "Failed to connect to Arduino on " << serialPort << std::endl;
		// return 1;
	}

	HttpServerHandler serverHandler(&arduino);
	serverHandler.Init();

	if (!serverHandler.Bind())
	{
		std::cerr << "Failed to bind server to port 8080\n";
		return 1;
	}

	std::thread serverThread([&serverHandler]() {
		serverHandler.Start();
	});

	while (running)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::cout << "Stopping server...\n";
	serverHandler.Stop();

	if (serverThread.joinable())
		serverThread.join();

	std::cout << "System shut down gracefully." << std::endl;
	return 0;
}
