/**
 * ArduinoSerial.h
 *
 * Header file for the ArduinoSerial class which handles
 * serial communication with an Arduino for conveyor belt control.
 */

#ifndef ARDUINO_SERIAL_H
#define ARDUINO_SERIAL_H

#include <string>
#include <termios.h>
#include <atomic>
#include <mutex>
#include <thread>

class ArduinoSerial
{

private:
	int serialPort;
	struct termios tty;

	std::thread readerThread;
	std::atomic<bool> keepReading;
	std::mutex dataMutex;
	std::string latestDistance;
	void readLoop(); // Background reader thread

public:
	/**
	 * Constructor - initializes the serial connection
	 *
	 * @param portName The serial port to connect to (e.g., "/dev/ttyACM0")
	 */
	ArduinoSerial(const std::string &portName);

	/**
	 * Destructor - closes the serial connection
	 */
	~ArduinoSerial();

	/**
	 * Send a command to the Arduino and get the response
	 *
	 * @param command The command to send
	 * @return The response from Arduino, or an error message
	 */
	std::string sendCommand(const std::string &command);

	/**
	 * Check if the serial connection is valid
	 *
	 * @return true if connected, false otherwise
	 */
	bool isConnected() const;

	/**
	 * Get the current status of the Arduino
	 *
	 * @return A status message with speed and direction info
	 */
	std::string getStatus();

	/**
	 * Set speed by percentage (0-100%)
	 *
	 * @param percent Speed percentage (0-100)
	 * @return The response from Arduino
	 */
	std::string setSpeed(int percent);

	/**
	 * Immediate stop of the motor
	 *
	 * @return The response from Arduino
	 */
	std::string stopImmediate();

	/**
	 * Gradual stop of the motor
	 *
	 * @param rampRate The rate of deceleration (1-50)
	 * @return The response from Arduino
	 */
	std::string stopGradual(int rampRate);

	/**
	 * Start the motor with gradual acceleration
	 *
	 * @param rampRate The rate of acceleration (1-20)
	 * @return The response from Arduino
	 */
	std::string start(int rampRate = 5);

	/**
	 * Set direction of the motor
	 *
	 * @param direction 1 for forward, -1 for reverse
	 * @return The response from Arduino
	 */
	std::string setDirection(int direction);

	/**
	 * Set the servo angle (0-180 degrees)
	 *
	 * @param angle Servo angle
	 * @return The response from Arduino
	 */
	std::string setServoAngle(int angle);

	/**
	 * Get the latest distance reading received from Arduino
	 *
	 * @return Last received DISTANCE line
	 */
	std::string getLatestDistance();

	/**
	 * Reverse current direction of Arduino motor 
	 *
	 * @return Response from Arduino
	 */
	std::string reverseDirection();

	/**
	 * Set the servo angle to initial
	 *
	 * @return The response to Arduino
	 */
	inline std::string setServoInitialAngle() { return sendCommand("SERVO:" + std::to_string(45)); }
};

#endif // ARDUINO_SERIAL_H