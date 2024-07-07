#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "wifi.h"

Servo myServo;
WiFiServer server(80);

const uint8_t servoPin = 18;
const uint8_t btn1Pin = 21;
const uint8_t btn2Pin = 34;
const uint8_t ledBuiltinPin = 2;

void setup() {
  // Attach the servo to the specified pin
  myServo.attach(servoPin);
  
  // Assign button pins as input
  pinMode(btn1Pin, INPUT);
  pinMode(btn2Pin, INPUT);
  pinMode(ledBuiltinPin, OUTPUT);
  digitalWrite(ledBuiltinPin, HIGH);
  // Initialize serial communication for debugging
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //digitalWrite(ledBuiltinPin, WiFi.status() != WL_CONNECTED);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
