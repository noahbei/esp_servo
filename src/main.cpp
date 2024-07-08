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
const char* PARAM_DIRECTION = "direction";
const char* PARAM_STOP = "stop";

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

// if (!LittleFS.begin()) {
//     Serial.println("An error has occurred while mounting LittleFS");
//     return;
//   }
  
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

//   // Send a GET request to <IP>/get?message=<message>
//   server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
//       String message;
//       if (request->hasParam(PARAM_COUNTER)) {
//           message = request->getParam(PARAM_COUNTER)->value();
//       } else {
//           message = "No message sent";
//       }
//       request->send(200, "text/plain", "Hello, GET: " + message);
//   });

  // Send a POST request to <IP>/rotate with a form field message set to direction
  server.on("/rotate", HTTP_POST, [](AsyncWebServerRequest *request){
      String direction;
      if (request->hasParam(PARAM_DIRECTION, true)) {
          direction = request->getParam(PARAM_DIRECTION, true)->value();
          if (direction == "clockwise") {
            Serial.println("Turning clockwise");
            myServo.write(180);
          }
          else if (direction == "counterclockwise") {
            Serial.println("Turning counterclockwise");
            myServo.write(0);
          }
      } else {
          direction = "Invalid Direction";
      }
      request->send(200, "text/plain", "Hello, POST: " + direction);
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
