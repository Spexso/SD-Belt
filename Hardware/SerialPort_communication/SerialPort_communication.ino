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
const int RPWM = 11;
const int LPWM = 12;
const int R_EN = 7;
const int L_EN = 8;
const int trigPin = 4;
const int echoPin = 5;

// Motor control variables
int currentSpeed = 0;
int targetSpeed = 0;
int currentDirection = 1;
int rampRate = 5;
unsigned long lastRampTime = 0;
const int rampInterval = 50;

String inputString = "";
boolean stringComplete = false;

void setup()
{
  Serial.begin(9600);
  ServoMotor.attach(3);
  inputString.reserve(200);

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void loop()
{
  if (stringComplete)
  {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }

  if (currentSpeed != targetSpeed && millis() - lastRampTime >= rampInterval)
  {
    updateMotorSpeed();
    lastRampTime = millis();
  }

  static unsigned long lastDistanceTime = 0;
  const unsigned long distanceInterval = 500;

  if (millis() - lastDistanceTime > distanceInterval)
  {
    long distance = measureDistance();
    if (distance > 0)
    {
      Serial.print("DISTANCE:");
      Serial.print(distance);
      Serial.println(" cm");
    }
    lastDistanceTime = millis();
  }
}

void updateMotorSpeed()
{
  if (currentSpeed < targetSpeed)
  {
    currentSpeed = min(currentSpeed + rampRate, targetSpeed);
  }
  else if (currentSpeed > targetSpeed)
  {
    currentSpeed = max(currentSpeed - rampRate, targetSpeed);
  }
  applyMotorControl();
}

void applyMotorControl()
{
  if (currentSpeed == 0)
  {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
  }
  else if (currentDirection == 1)
  {
    analogWrite(LPWM, 0);
    analogWrite(RPWM, currentSpeed);
  }
  else if (currentDirection == -1)
  {
    analogWrite(LPWM, currentSpeed);
    analogWrite(RPWM, 0);
  }
  else
  {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, currentSpeed);
  }
}

void processCommand(String command)
{
  command.trim();

  int colonIndex = command.indexOf(':');
  String cmd;
  String valueStr = "";
  int value = 0;

  if (colonIndex != -1)
  {
    cmd = command.substring(0, colonIndex);
    valueStr = command.substring(colonIndex + 1);
    valueStr.trim();
    Serial.print("→ valueStr before toInt: '");
    Serial.print(valueStr);
    Serial.println("'");
    value = valueStr.toInt();
  }
  else
  {
    cmd = command;
  }

  if (cmd == "PCT")
  {
    if (value >= 0 && value <= 100)
    {
      targetSpeed = map(value, 0, 100, 0, 255);
      Serial.print("OK:PCT:");
      Serial.print(value);
      Serial.println("%");
    }
    else
    {
      Serial.println("ERR:Invalid percentage. Use 0-100");
    }
  }
  else if (cmd == "STOP")
  {
    if (valueStr == "" || value == 0)
    {
      targetSpeed = 0;
      currentSpeed = 0;
      applyMotorControl();
      Serial.println("OK:STOP:Immediate");
    }
    else
    {
      targetSpeed = 0;
      rampRate = constrain(value, 1, 50);
      Serial.print("OK:STOP:Gradual:");
      Serial.println(rampRate);
    }
  }
  else if (cmd == "START")
  {
    rampRate = (valueStr == "" || value == 0) ? 5 : constrain(value, 1, 20);
    Serial.print("OK:START:RampRate:");
    Serial.println(rampRate);
  }
  else if (cmd == "DIR")
  {
    if (value == 1 || value == -1)
    {
      currentDirection = value;
      applyMotorControl();
      Serial.println(currentDirection == 1 ? "OK:DIR:Forward" : "OK:DIR:Reverse");
    }
    else
    {
      Serial.println("ERR:Invalid direction. Use 1 (forward) or -1 (reverse)");
    }
  }
  else if (cmd == "STATUS")
  {
    String dirText = (currentDirection == 1) ? "Forward" : "Reverse";
    int pctSpeed = map(currentSpeed, 0, 255, 0, 100);
    Serial.print("STATUS:Speed:");
    Serial.print(pctSpeed);
    Serial.print("%:Direction:");
    Serial.println(dirText);
  }
  else if (cmd == "SERVO")
  {
    if (value >= 0 && value <= 180)
    {
      ServoMotor.write(value);
      Serial.print("OK:SERVO:");
      Serial.println(value);
    }
  }
  else if(cmd == "REV")
  {
    currentDirection *= -1;
    applyMotorControl();
    Serial.println(currentDirection == 1 ? "OK:DIR:Forward" : "OK:DIR:Reverse");
  }
  else
  {
    Serial.println("ERR:Unknown command");
  }
}

// Distance measurement function
long measureDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0)
    return -1;
  long distance = duration * 0.034 / 2;
  return distance;
}

// Serial input handler
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (inChar != '\n')
    {
      inputString += inChar;
    }
    if (inChar == '\n')
    {
      stringComplete = true;
    }
  }
}
