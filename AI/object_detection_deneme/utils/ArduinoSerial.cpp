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
#include <algorithm>
#include <mutex>

// Constructor
ArduinoSerial::ArduinoSerial(const std::string &portName)
{
	// Open the serial port
	serialPort = open(portName.c_str(), O_RDWR);

	if (serialPort < 0)
	{
		std::cerr << "Error opening serial port: " << strerror(errno) << std::endl;
		return;
	}

	// Get current serial port settings
	if (tcgetattr(serialPort, &tty) != 0)
	{
		std::cerr << "Error getting serial port attributes: " << strerror(errno) << std::endl;
		close(serialPort);
		serialPort = -1;
		return;
	}

	// Set Baud Rate
	cfsetospeed(&tty, B9600);
	cfsetispeed(&tty, B9600);

	// Serial port configuration
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL;

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG;

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;

	// Timeouts
	tty.c_cc[VTIME] = 10;
	tty.c_cc[VMIN] = 0;

	// Apply settings
	if (tcsetattr(serialPort, TCSANOW, &tty) != 0)
	{
		std::cerr << "Error setting serial port attributes: " << strerror(errno) << std::endl;
		close(serialPort);
		serialPort = -1;
		return;
	}

	// Give Arduino time to reset
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "Serial port initialized" << std::endl;

	// Start background reading
	keepReading = true;
	readerThread = std::thread(&ArduinoSerial::readLoop, this);
}

// Destructor
ArduinoSerial::~ArduinoSerial()
{
	// Stop background thread
	keepReading = false;
	if (readerThread.joinable())
		readerThread.join();

	if (serialPort >= 0)
	{
		close(serialPort);
		std::cout << "Serial port closed" << std::endl;
	}
}

// Background thread to read data from Arduino
void ArduinoSerial::readLoop()
{
	char buffer[256];

	while (keepReading)
	{
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytesRead = read(serialPort, buffer, sizeof(buffer) - 1);
		if (bytesRead > 0)
		{
			std::string line(buffer);
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

			if (line.find("DISTANCE:") != std::string::npos)
			{
				std::lock_guard<std::mutex> lock(dataMutex);
				latestDistance = line;
				std::cout << "↪️ Incoming: " << line << std::endl;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

// Send a command to Arduino and wait for a response
std::string ArduinoSerial::sendCommand(const std::string &command)
{
	if (serialPort < 0)
	{
		return "Error: Serial port not open";
	}

	std::cout << "Sending command: " << command << std::endl;

	std::string fullCommand = command + "\n";

	ssize_t bytesWritten = write(serialPort, fullCommand.c_str(), fullCommand.length());
	if (bytesWritten < 0)
	{
		return "Error writing to serial port: " + std::string(strerror(errno));
	}

	// Read loop until newline or timeout
	std::string response;
	char ch;
	auto start = std::chrono::steady_clock::now();

	while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
	{
		ssize_t result = read(serialPort, &ch, 1);
		if (result > 0)
		{
			if (ch == '\n')
			{
				break;
			}
			response += ch;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	if (response.empty())
	{
		return "Error: No response or timeout";
	}

	std::cout << "Response: " << response << std::endl;
	return response;
}

// Check if the serial connection is valid
bool ArduinoSerial::isConnected() const
{
	return serialPort >= 0;
}

// Get the current status of the Arduino
std::string ArduinoSerial::getStatus()
{
	return sendCommand("STATUS");
}

// Set speed by percentage (0-100%)
std::string ArduinoSerial::setSpeed(int percent)
{
	if (percent < 0 || percent > 100)
	{
		return "Error: Speed percentage must be between 0 and 100";
	}
	return sendCommand("PCT:" + std::to_string(percent));
}

// Immediate stop of the motor
std::string ArduinoSerial::stopImmediate()
{
	return sendCommand("STOP:0");
}

// Gradual stop of the motor
std::string ArduinoSerial::stopGradual(int rampRate)
{
	if (rampRate < 1 || rampRate > 50)
	{
		return "Error: Ramp rate must be between 1 and 50";
	}
	return sendCommand("STOP:" + std::to_string(rampRate));
}

// Start the motor with gradual acceleration
std::string ArduinoSerial::start(int rampRate)
{
	if (rampRate < 1 || rampRate > 20)
	{
		return "Error: Ramp rate must be between 1 and 20";
	}
	return sendCommand("START:" + std::to_string(rampRate));
}

// Set direction of the motor
std::string ArduinoSerial::setDirection(int direction)
{
	if (direction != 1 && direction != -1)
	{
		return "Error: Direction must be 1 (forward) or -1 (reverse)";
	}

	std::string command = "DIR:" + std::to_string(direction);
	std::string fullCommand = command + "\n";

	std::cout << "Raw bytes being sent: ";
	for (char c : fullCommand)
	{
		std::cout << "[" << static_cast<int>(c) << "]";
	}
	std::cout << std::endl;

	return sendCommand(command);
}

// Reverse the direction of the motor
std::string ArduinoSerial::reverseDirection()
{
	std::string command = "REV";
	
	return sendCommand(command);
}


// Set the servo angle (0–180)
std::string ArduinoSerial::setServoAngle(int angle)
{
	if (angle < 0 || angle > 180)
	{
		return "Error: Servo angle must be between 0 and 180";
	}
	return sendCommand("SERVO:" + std::to_string(angle));
}

// Return the most recently received distance
std::string ArduinoSerial::getLatestDistance()
{
	std::lock_guard<std::mutex> lock(dataMutex);
	return latestDistance;
}
