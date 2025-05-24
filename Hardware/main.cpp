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

std::atomic<bool> run_ard(true);

// Signal handler
void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	run_ard = false;
}

class HttpServerHandler
{
public:
	HttpServerHandler(ArduinoSerial *arduinoPtr)
		: arduino(arduinoPtr) {}

	void Init()
	{
		// Setup logger to print every request
		server.set_logger([](const httplib::Request &req, const httplib::Response &res)
						  {
				std::cout << "[HTTP] " << req.method << " " << req.path
						  << " => " << res.status << std::endl;
		
				if (!req.body.empty())
				{
					std::cout << "  Body: " << req.body << std::endl;
				} });

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

		server.Post("/speed", [this](const httplib::Request &req, httplib::Response &res)
					{
					std::string body = req.body;
					std::cout << "[/speed] Received body: " << body << std::endl;
				
					try
					{
						int percent = std::stoi(body);
						std::string response = arduino->setSpeed(percent);
						std::cout << "[/speed] Response: " << response << std::endl;
						res.set_content(response, "text/plain");
					}
					catch (const std::exception &e)
					{
						std::string error = std::string("ERR: Invalid speed value - ") + e.what();
						std::cerr << "[/speed] " << error << std::endl;
						res.set_content(error, "text/plain");
						res.status = 400;
					} });
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

		try
		{
			if (cmd == "start")
				return arduino->start(5); // Example ramp rate

			if (cmd == "stop")
				return arduino->stopImmediate();

			if (cmd == "reverse")
				return arduino->setDirection(-1);

			if (cmd == "status")
				return arduino->getStatus();

			if (cmd.rfind("speed=", 0) == 0)
			{
				int pct = std::stoi(cmd.substr(6));
				return arduino->setSpeed(pct);
			}

			if (cmd.rfind("servo=", 0) == 0)
			{
				int angle = std::stoi(cmd.substr(6));
				return arduino->setServoAngle(angle);
			}

			if (cmd == "dir=1")
				return arduino->setDirection(1);

			if (cmd == "dir=-1")
				return arduino->setDirection(-1);

			return "ERR: Unknown command";
		}
		catch (const std::exception &e)
		{
			return std::string("ERR: Invalid argument - ") + e.what();
		}
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

	std::thread serverThread([&serverHandler]()
							 { serverHandler.Start(); });

	while (run_ard)
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
