#include <Arduino.h>
#include <ESP32Servo.h>

Servo myServo;

const int servoPin = 18;
const int btn1Pin = 21;
const int btn2Pin = 34;

void setup() {
  // Attach the servo to the specified pin
  myServo.attach(servoPin);
  
  // Assign button pins as input
  pinMode(btn1Pin, INPUT);
  pinMode(btn2Pin, INPUT);
  
  // Initialize serial communication for debugging
  Serial.begin(115200);
}

void loop() {
  // Read button states
  int btn1State = digitalRead(btn1Pin);
  int btn2State = digitalRead(btn2Pin);
  
  // Control servo based on button states
  if (btn1State == HIGH) {
    Serial.println("Turning counterclockwise");
    myServo.write(0); // Set servo to counterclockwise direction
  } else if (btn2State == HIGH) { 
    Serial.println("Turning clockwise");
    myServo.write(180); // Set servo to clockwise direction
  } else {
    Serial.println("Stopping");
    myServo.write(92); // Stop servo at midpoint
  }
  
  delay(50); // Delay to debounce and stabilize readings
}
