/*
 * Arduino Conveyor Belt Controller
 * Features:
 * - Percentage-based speed control (0-100%)
 * - Immediate and gradual stop options
 * - Gradual speed increase
 * - Reverse direction control
 * - Serial communication with Raspberry Pi
 */

 #include <Servo.h>

 Servo ServoMotor;
 
 // BTS7960 Motor Controller Pins
 const int RPWM = 11;    // PWM signal for clockwise rotation (forward)
 const int LPWM = 12;   // PWM signal for counter-clockwise rotation (reverse)
 const int R_EN = 7;    // Enable pin for clockwise rotation
 const int L_EN = 8;    // Enable pin for counter-clockwise rotation
 
 // Motor control variables
 int currentSpeed = 0;        // Current speed (0-255)
 int targetSpeed = 0;         // Target speed for acceleration/deceleration (0-255)
 int currentDirection = 1;    // 1 for forward, -1 for reverse
 int rampRate = 5;            // Speed change per iteration during ramping (1-20)
 unsigned long lastRampTime = 0;  // Last time speed was adjusted
 const int rampInterval = 50; // Milliseconds between speed adjustments
 
 String inputString = "";      // String to hold incoming data
 boolean stringComplete = false;  // Whether the string is complete
 
 void setup() {
   // Initialize serial communication
   Serial.begin(9600);
 
   // Servo Motor Signal pin
   ServoMotor.attach(3);  
   
   // Reserve memory for inputString
   inputString.reserve(200);
   
   // Configure motor controller pins
   pinMode(RPWM, OUTPUT);
   pinMode(LPWM, OUTPUT);
   pinMode(R_EN, OUTPUT);
   pinMode(L_EN, OUTPUT);
   
   // Enable the motor driver
   digitalWrite(R_EN, HIGH);
   digitalWrite(L_EN, HIGH);
   
   // Initially stop the motor
   analogWrite(RPWM, 0);
   analogWrite(LPWM, 0);
 }
 
 void loop() {
   // Process commands when a complete string is received
   if (stringComplete) {
     processCommand(inputString);
     
     // Clear the string for the next command
     inputString = "";
     stringComplete = false;
   }
   
   // Handle gradual speed changes (acceleration/deceleration)
   if (currentSpeed != targetSpeed && millis() - lastRampTime >= rampInterval) {
     updateMotorSpeed();
     lastRampTime = millis();
   }
 }
 
 // Update motor speed during gradual acceleration/deceleration
 void updateMotorSpeed() {
   if (currentSpeed < targetSpeed) {
     // Accelerate
     currentSpeed = min(currentSpeed + rampRate, targetSpeed);
   } else if (currentSpeed > targetSpeed) {
     // Decelerate
     currentSpeed = max(currentSpeed - rampRate, targetSpeed);
   }
   
   // Apply the updated speed to the motor
   applyMotorControl();
 }
 
 // Apply current speed and direction to the motor
 void applyMotorControl() {
   if (currentSpeed == 0) {
     // Stop motor
     analogWrite(RPWM, 0);
     analogWrite(LPWM, 0);
   } else if (currentDirection == 1) {
     // Forward direction
     analogWrite(LPWM, 0);  // Ensure reverse is off
     analogWrite(RPWM, currentSpeed);
   } else {
     // Reverse direction
     analogWrite(RPWM, 0);  // Ensure forward is off
     analogWrite(LPWM, currentSpeed);
   }
 }
 
 // Process serial commands
 void processCommand(String command) {
   // Remove any whitespace
   command.trim();
   
   // Split command and value
   int colonIndex = command.indexOf(':');
   String cmd;
   String valueStr = "";
   int value = 0;
   
   if (colonIndex != -1) {
     cmd = command.substring(0, colonIndex);
     valueStr = command.substring(colonIndex + 1);
     value = valueStr.toInt();
   } else {
     cmd = command;
   }
   
   // Process commands
   if (cmd == "PCT") {
     // Set speed by percentage (0-100%)
     if (value >= 0 && value <= 100) {
       targetSpeed = map(value, 0, 100, 0, 255);
       Serial.print("OK:PCT:");
       Serial.print(value);
       Serial.println("%");
     } else {
       Serial.println("ERR:Invalid percentage. Use 0-100");
     }
   }
   else if (cmd == "STOP") {
     if (valueStr == "" || value == 0) {
       // Immediate stop
       targetSpeed = 0;
       currentSpeed = 0;  // Bypass the ramp
       applyMotorControl();
       Serial.println("OK:STOP:Immediate");
     } else {
       // Gradual stop with custom ramp rate
       targetSpeed = 0;
       rampRate = constrain(value, 1, 50);  // Constrain ramp rate for safety
       Serial.print("OK:STOP:Gradual:");
       Serial.println(rampRate);
     }
   }
   else if (cmd == "START") {
     if (valueStr == "" || value == 0) {
       // Default start with standard ramp rate
       rampRate = 5;
     } else {
       // Start with custom ramp rate
       rampRate = constrain(value, 1, 20);
     }
     Serial.print("OK:START:RampRate:");
     Serial.println(rampRate);
   }
   else if (cmd == "DIR") {
     // Set direction: 1=forward, -1=reverse
     if (value == 1 || value == -1) {
       currentDirection = value;
       applyMotorControl();
       if (currentDirection == 1) {
         Serial.println("OK:DIR:Forward");
       } else {
         Serial.println("OK:DIR:Reverse");
       }
     } else {
       Serial.println("ERR:Invalid direction. Use 1 (forward) or -1 (reverse)");
     }
   }
   else if (cmd == "REV") {
     // Toggle direction
     currentDirection = -currentDirection;
     applyMotorControl();
     if (currentDirection == 1) {
       Serial.println("OK:REV:Forward");
     } else {
       Serial.println("OK:REV:Reverse");
     }
   }
   else if (cmd == "STATUS") {
     // Report current status
     String dirText = (currentDirection == 1) ? "Forward" : "Reverse";
     int pctSpeed = map(currentSpeed, 0, 255, 0, 100);
     Serial.print("STATUS:Speed:");
     Serial.print(pctSpeed);
     Serial.print("%:Direction:");
     Serial.println(dirText);
   }
   else if (cmd == "SERVO") {
     if (value >= 0 && value <= 180) {
       ServoMotor.write(value);
       Serial.print("OK:SERVO:");
       Serial.println(value);
     } 
   }
   else {
     Serial.println("ERR:Unknown command");
   }
 }
 
 // Event handler for serial data
 void serialEvent() {
   while (Serial.available()) {
     char inChar = (char)Serial.read();
     
     // Add received character to input string
     if (inChar != '\n') {
       inputString += inChar;
     }
     
     // If newline is received, set a flag so the main loop can process the command
     if (inChar == '\n') {
       stringComplete = true;
     }
   }
 }