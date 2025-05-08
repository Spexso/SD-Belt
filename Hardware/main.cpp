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
 
	 void Run()
	 {
		 httplib::Server server;
 
		 server.Get("/status", [](const httplib::Request &, httplib::Response &res) {
			 res.set_content("Server is running!", "text/plain");
		 });
 
		 server.Get("/command", [this](const httplib::Request &req, httplib::Response &res) {
			 if (!req.has_param("cmd"))
			 {
				 res.status = 400;
				 res.set_content("Missing 'cmd' parameter", "text/plain");
				 return;
			 }
			 std::string cmd = req.get_param_value("cmd");
			 std::string result = handleCommand(cmd);
			 res.set_content(result, "text/plain");
		 });
 
		 std::cout << "API SERVER running at http://0.0.0.0:8080\n";
		 server.listen("0.0.0.0", 8080);
	 }
 
 private:
	 ArduinoSerial *arduino;
 
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
 
	 std::cout << "Starting conveyor belt controller" << std::endl;
	 std::cout << "Using serial port: " << serialPort << std::endl;
 
	 ArduinoSerial arduino(serialPort);
	 if (!arduino.isConnected())
	 {
		 std::cerr << "Failed to connect to Arduino on " << serialPort << std::endl;
		 return 1;
	 }
 
	 HttpServerHandler serverHandler(&arduino);
	 std::thread serverThread([&serverHandler]() {
		 serverHandler.Run();
	 });
 
	 while (running)
	 {
		 std::this_thread::sleep_for(std::chrono::seconds(1));
	 }
 
	 if (serverThread.joinable())
		 serverThread.join();
 
	 std::cout << "System shut down gracefully." << std::endl;
	 return 0;
 }
 