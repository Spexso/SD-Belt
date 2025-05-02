/**
 * ArduinoSerial.cpp
 * 
 * Implementation file for the ArduinoSerial class which handles
 * serial communication with an Arduino for conveyor belt control.
 */

 #include "ArduinoSerial.h"
 #include <iostream>
 #include <fcntl.h>
 #include <unistd.h>
 #include <cstring>
 #include <thread>
 #include <chrono>
 
 // Constructor
 ArduinoSerial::ArduinoSerial(const std::string& portName) {
	 // Open the serial port
	 serialPort = open(portName.c_str(), O_RDWR);
	 
	 if (serialPort < 0) {
		 std::cerr << "Error opening serial port: " << strerror(errno) << std::endl;
		 return;
	 }
	 
	 // Get current serial port settings
	 if (tcgetattr(serialPort, &tty) != 0) {
		 std::cerr << "Error getting serial port attributes: " << strerror(errno) << std::endl;
		 close(serialPort);
		 serialPort = -1;
		 return;
	 }
	 
	 // Set Baud Rate
	 cfsetospeed(&tty, B9600);
	 cfsetispeed(&tty, B9600);
	 
	 // Setting other port settings
	 tty.c_cflag &= ~PARENB;        // No parity
	 tty.c_cflag &= ~CSTOPB;        // 1 stop bit
	 tty.c_cflag &= ~CSIZE;         // Clear size bits
	 tty.c_cflag |= CS8;            // 8 bits per byte
	 tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
	 tty.c_cflag |= CREAD | CLOCAL; // Enable READ & ignore ctrl lines
	 
	 tty.c_lflag &= ~ICANON;        // Non-canonical mode
	 tty.c_lflag &= ~ECHO;          // Disable echo
	 tty.c_lflag &= ~ECHOE;         // Disable erasure
	 tty.c_lflag &= ~ECHONL;        // Disable new-line echo
	 tty.c_lflag &= ~ISIG;          // Disable interpretation of INTR, QUIT and SUSP
	 
	 tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	 tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable special handling of bytes
	 
	 tty.c_oflag &= ~OPOST;         // Prevent special interpretation of output bytes
	 tty.c_oflag &= ~ONLCR;         // Prevent conversion of newline to carriage return/line feed
	 
	 // Setting read timeouts
	 tty.c_cc[VTIME] = 10;          // Wait for up to 1 second
	 tty.c_cc[VMIN] = 0;            // No minimum number of characters
	 
	 // Save settings
	 if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
		 std::cerr << "Error setting serial port attributes: " << strerror(errno) << std::endl;
		 close(serialPort);
		 serialPort = -1;
		 return;
	 }
	 
	 // Give Arduino time to reset after opening the port
	 std::this_thread::sleep_for(std::chrono::seconds(2));
	 std::cout << "Serial port initialized" << std::endl;
 }
 
 // Destructor
 ArduinoSerial::~ArduinoSerial() {
	 if (serialPort >= 0) {
		 close(serialPort);
		 std::cout << "Serial port closed" << std::endl;
	 }
 }
 
 // Send a command to Arduino
 std::string ArduinoSerial::sendCommand(const std::string& command) {
	 if (serialPort < 0) {
		 return "Error: Serial port not open";
	 }
	 
	 // Log the command being sent
	 std::cout << "Sending command: " << command << std::endl;
	 
	 // Add newline to command
	 std::string fullCommand = command + "\n";
	 
	 // Write to serial port
	 ssize_t bytesWritten = write(serialPort, fullCommand.c_str(), fullCommand.length());
	 if (bytesWritten < 0) {
		 return "Error writing to serial port: " + std::string(strerror(errno));
	 }
	 
	 // Wait a moment for Arduino to process
	 std::this_thread::sleep_for(std::chrono::milliseconds(100));
	 
	 // Read response
	 char buffer[256];
	 memset(buffer, 0, sizeof(buffer));
	 ssize_t bytesRead = read(serialPort, buffer, sizeof(buffer) - 1);
	 
	 if (bytesRead < 0) {
		 return "Error reading from serial port: " + std::string(strerror(errno));
	 }
	 
	 // Log the response
	 std::string response(buffer);
	 std::cout << "Response: " << response << std::endl;
	 
	 // Return the response string
	 return response;
 }
 
 // Check if serial connection is valid
 bool ArduinoSerial::isConnected() const {
	 return serialPort >= 0;
 }
 
 // Get the current status of the Arduino
 std::string ArduinoSerial::getStatus() {
	 return sendCommand("STATUS");
 }
 
 // Set speed by percentage (0-100%)
 std::string ArduinoSerial::setSpeed(int percent) {
	 // Validate percentage
	 if (percent < 0 || percent > 100) {
		 return "Error: Speed percentage must be between 0 and 100";
	 }
	 
	 // Send command
	 return sendCommand("PCT:" + std::to_string(percent));
 }
 
 // Immediate stop of the motor
 std::string ArduinoSerial::stopImmediate() {
	 return sendCommand("STOP:0");
 }
 
 // Gradual stop of the motor
 std::string ArduinoSerial::stopGradual(int rampRate) {
	 // Validate ramp rate
	 if (rampRate < 1 || rampRate > 50) {
		 return "Error: Ramp rate must be between 1 and 50";
	 }
	 
	 // Send command
	 return sendCommand("STOP:" + std::to_string(rampRate));
 }
 
 // Start the motor with gradual acceleration
 std::string ArduinoSerial::start(int rampRate) {
	 // Validate ramp rate
	 if (rampRate < 1 || rampRate > 20) {
		 return "Error: Ramp rate must be between 1 and 20";
	 }
	 
	 // Send command
	 return sendCommand("START:" + std::to_string(rampRate));
 }
 
 // Set direction of the motor
 std::string ArduinoSerial::setDirection(int direction) {
	 // Validate direction
	 if (direction != 1 && direction != -1) {
		 return "Error: Direction must be 1 (forward) or -1 (reverse)";
	 }
	 
	 // Send command
	 return sendCommand("DIR:" + std::to_string(direction));
 }
 
 // Reverse the current direction of the motor
 std::string ArduinoSerial::reverseDirection() {
	 return sendCommand("REV");
 }

 std::string ArduinoSerial::setServoAngle(int angle)
 {
	 if (angle < 0 || angle > 180) {
		 return "Error: Servo angle must be between 0 and 180";
	 }
 
	 return sendCommand("SERVO:" + std::to_string(angle));
 }