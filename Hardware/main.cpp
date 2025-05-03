/**
 * main.cpp
 * 
 * Main program for controlling a conveyor belt via Arduino.
 * Implements a specific scenario sequence for testing the system.
 */

 #include "ArduinoSerial.h"
 #include <iostream>
 #include <chrono>
 #include <thread>
 #include <csignal>
 
 
 // Signal handler flag for clean shutdown
 volatile sig_atomic_t running = 1;
 
 // Signal handler function
 void signalHandler(int signum) {
	 std::cout << "Interrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	 running = 0;
 }
 
 int main(int argc, char* argv[]) {
	 // Set up signal handling for clean shutdown
	 signal(SIGINT, signalHandler);
	 signal(SIGTERM, signalHandler);
 
	 // Default serial port
	 std::string serialPort = "/dev/ttyUSB0";
 
	 // Check command line arguments for serial port
	 if (argc > 1) {
		 serialPort = argv[1];
	 }
 
	 std::cout << "Starting conveyor belt controller" << std::endl;
	 std::cout << "Using serial port: " << serialPort << std::endl;
 
	 // Create Arduino serial connection
	 ArduinoSerial arduino(serialPort);
 
	 if (!arduino.isConnected()) {
		 std::cerr << "Failed to connect to Arduino on " << serialPort << std::endl;
		 return 1;
	 }
 
	 try {
		 std::cout << "\n=== Starting Conveyor Belt Scenario ===\n" << std::endl;
 
		 // Set initial servo position
		 std::cout << "\n=== Setting initial angle for Servo Motor ===\n" << std::endl;
		 arduino.setServoInitialAngle();
 
		 // Step 1: Start motor with gentle ramp rate
		 std::cout << "Step 1: Starting motor..." << std::endl;
		 arduino.start(3);
		 arduino.setDirection(1);
 
		 // Step 2: Gradually increase speed in 5 steps
		 int speedIncrement = 20;
		 for (int step = 1; step <= 5 && running; step++) {
			 int speed = step * speedIncrement;
			 std::cout << "Step 2." << step << ": Increasing speed to " << speed << "%" << std::endl;
			 arduino.setSpeed(speed);
			 std::this_thread::sleep_for(std::chrono::seconds(2));
		 }
 
		 if (!running) {
			 std::cout << "Scenario interrupted. Stopping motor..." << std::endl;
			 arduino.stopImmediate();
			 return 0;
		 }
 
		 // Step 3: Run at max speed for 10 seconds with distance checks
		 std::cout << "Step 3: Running at max speed for 10 seconds..." << std::endl;
		 for (int i = 0; i < 10 && running; i++) {
			 std::string distanceLine = arduino.ge
			 tLatestDistance();
 
			 if (!distanceLine.empty()) {
				 std::cout << "  Distance reading: " << distanceLine << std::endl;
 
				 // Extract numeric value from "DISTANCE:XX cm"
				 std::size_t pos = distanceLine.find(":");
				 if (pos != std::string::npos) {
					 int dist = std::stoi(distanceLine.substr(pos + 1));
					 if (dist > 0 && dist < 10) {
						 std::cout << "⚠️  Object too close (" << dist << " cm)! Stopping motor." << std::endl;
						 arduino.stopImmediate();
						 return 0;
					 }
				 }
			 }
 
			 std::cout << "  Running... " << (i + 1) << " seconds" << std::endl;
			 std::this_thread::sleep_for(std::chrono::seconds(1));
		 }
 
		 if (!running) {
			 std::cout << "Scenario interrupted. Stopping motor..." << std::endl;
			 arduino.stopImmediate();
			 return 0;
		 }
 
		 // Step 4: Decrease speed gradually to 60%
		 std::cout << "Step 4: Gradually decreasing speed to 60%..." << std::endl;
		 arduino.setSpeed(80);
		 std::this_thread::sleep_for(std::chrono::seconds(2));
 
		 if (!running) {
			 std::cout << "Scenario interrupted. Stopping motor..." << std::endl;
			 arduino.stopImmediate();
			 return 0;
		 }
 
		 arduino.setSpeed(60);
		 std::this_thread::sleep_for(std::chrono::seconds(2));
 
		 // Servo interaction
		 std::cout << "Moving servo to 90 degrees..." << std::endl;
		 arduino.setServoAngle(90);
		 std::this_thread::sleep_for(std::chrono::seconds(2));
 
		 std::cout << "Resetting servo to 0 degrees..." << std::endl;
		 arduino.setServoAngle(0);
		 std::this_thread::sleep_for(std::chrono::seconds(2));
 
		 // Step 5: Stop motor
		 std::cout << "Step 5: Stopping motor immediately..." << std::endl;
		 arduino.stopImmediate();
 
		 std::cout << "\n=== Scenario Completed Successfully ===\n" << std::endl;
 
	 } catch (const std::exception& e) {
		 std::cerr << "Error: " << e.what() << std::endl;
		 arduino.stopImmediate();
		 return 1;
	 }
 
	 return 0;
 }
 