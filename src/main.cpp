#include <Arduino.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "wifi-config.h"

Servo myServo;
AsyncWebServer server(80);

const uint8_t servoPin = 18;
const uint8_t btn1Pin = 21;
const uint8_t btn2Pin = 34;
const uint8_t ledBuiltinPin = 2;
const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  // Attach the servo to the specified pin
  myServo.attach(servoPin);
  
  // Assign button pins as input
  pinMode(btn1Pin, INPUT);
  pinMode(btn2Pin, INPUT);
  pinMode(ledBuiltinPin, OUTPUT);
  
  // Initialize serial communication for debugging
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Hello, world");
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String message;
      if (request->hasParam(PARAM_MESSAGE)) {
          message = request->getParam(PARAM_MESSAGE)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM_MESSAGE, true)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop() {


}

// void loop() {
//   // Read button states
//   int btn1State = digitalRead(btn1Pin);
//   int btn2State = digitalRead(btn2Pin);
  
//   // Control servo based on button states
//   if (btn1State == HIGH) {
//     Serial.println("Turning counterclockwise");
//     myServo.write(0); // Set servo to counterclockwise direction
//   } else if (btn2State == HIGH) { 
//     Serial.println("Turning clockwise");
//     myServo.write(180); // Set servo to clockwise direction
//   } else {
//     //Serial.println("Stopping");
//     myServo.write(92); // Stop servo at midpoint
//   }
  
//   delay(50); // Delay to debounce and stabilize readings
// }
