#include <Arduino.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <Button.h>
#include "main.h"
#include "wifi-config.h"
#include "MagneticEncoder.h"

// object definitions
Servo myServo;
AsyncWebServer server(80);
Button button1(btn1Pin);
Button button2(btn2Pin);

const char* PARAM_DIRECTION = "direction";
const char* PARAM_STOP = "stop";
enum windowTransitionState {
  WIN_TRANSITION_CLOSE,
  WIN_TRANSITION_OPEN,
  WIN_ROTATE_CLOSE,
  WIN_ROTATE_OPEN,
  WIN_STOP
};
windowTransitionState rotationState = WIN_STOP;

enum windowEndState = {
  OPEN = 0,
  CLOSE,
  MIDDLE
} endState;
// open state could potentially be updated when we are between the marging and edge of max rotation interval.

float globalAngle = 0;
const int maxRotationInterval[] = {0, 900}; // max rotation range in degrees

void setup()
{
  Serial.begin(115200);

  // peripheral initialization
  pinMode(ledBuiltinPin, OUTPUT);
  myServo.attach(servoPin);
  setupEncoder();
  button1.begin();
  button2.begin();
  wifiSetup();

  // endpoint initialization
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hello, world"); });
  server.on("/rotate", HTTP_POST, handleRotateRequest);
  server.on("/curtain", HTTP_POST, handleCurtainRequest);
  server.on("/stop", HTTP_POST, handleStopRequest);
  server.on("/resetPosition", HTTP_POST, handleResetRequest);
  server.on("/status", HTTP_GET, handleGetStatusRequest);

  server.onNotFound(notFound);
  server.begin();
}

bool flag = true;
void loop()
{
  digitalWrite(ledBuiltinPin, WiFi.isConnected()); // update to only check once every 10 seconds
  globalAngle = updateRotation();

  // can split this up in a way where overrides makes sense for control
  // make sure that there isn't something being changed by the post requests before this code
  if (!(rotationState == WIN_TRANSITION_OPEN || rotationState == WIN_TRANSITION_CLOSE)) {
    if (button1.pressed()) rotationState = WIN_ROTATE_OPEN;
    else if (button2.pressed()) rotationState = WIN_ROTATE_CLOSE;
    else if (button1.released() || button2.released()) rotationState = WIN_STOP;
  }
  else {

  }
  // make sure window can't be rotated out of bounds
  if ((globalAngle >= maxRotationInterval[1] - MARGIN && rotState == ROT_LEFT) ||
      (globalAngle <= maxRotationInterval[0] + MARGIN && rotState == ROT_RIGHT))
  {
    rotationState = WIN_STOP;
  }
  // lock control to rotate open/closed when win_transition_x is active
  switch (rotationState) {
    case WIN_TRANSITION_OPEN:
    case WIN_ROTATE_OPEN:
      myServo.write(SERVO_ROT_CLOCK);
      serial.println("rotating open");
      break;
    case WIN_TRANSITION_CLOSE:
    case WIN_ROTATE_CLOSE:
      myServo.write(SERVO_ROT_COUNTER);
      serial.println("rotating close");
      break;
    case WIN_STOP:
      myServo.write(SERVO_STOP);
      Serial.println("rotation stopped");
      break;
    default:
      myServo.write(SERVO_STOP);
      Serial.println("default behavior: rotation stopped");
      break;
  }

  // if (rotState)
  // {
  //   if ((globalAngle >= maxRotationInterval[1] - MARGIN && rotState == ROT_LEFT) || (globalAngle <= maxRotationInterval[0] + MARGIN && rotState == ROT_RIGHT))
  //   {
  //     myServo.write(SERVO_STOP);
  //     rotState = STOP;
  //   }
  // }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

/**
 * @brief <IP>/curtain route handler
 * 
 * Send a POST request to <IP>/curtain with a form field message set to direction
 */
void handleCurtainRequest(AsyncWebServerRequest *request)
{
  String direction;
  if (state == WIN_TRANSITION_CLOSE || state == WIN_TRANSITION_OPEN)
  {
    request->send(403, "text/plain", "Cannot control encoder, current state: " + String(state));
    return;
  }

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
    if (direction == "open")
    {
      Serial.println("Opening Window");
      state = WIN_TRANSITION_OPEN;
    }
    else if (direction == "close")
    {
      Serial.println("Closing Window");
      state = WIN_TRANSITION_CLOSE;
    }
    else
    {
      direction = "Invalid Direction";
    }
  }
  else
  {
    direction = "Invalid Direction";
  }
  request->send(200, "text/plain", "Hello, POST: " + direction);
}

/**
 * @brief <IP>/rotate route handler
 * 
 * Send a POST request to <IP>/rotate with a form field message set to direction
 */
void handleRotateRequest(AsyncWebServerRequest *request)
{
  String direction;
  if (state == WIN_TRANSITION_CLOSE || state == WIN_TRANSITION_OPEN)
  {
    request->send(403, "text/plain", "Cannot control encoder, current state: " + String(state));
    return;
  }

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
    if (direction == "clockwise")
    {
      Serial.println("Turning clockwise");
      rotState = ROT_RIGHT;
      myServo.write(SERVO_ROT_COUNTER);
    }
    else if (direction == "counterclockwise")
    {
      Serial.println("Turning counterclockwise");
      rotState = ROT_LEFT;
      myServo.write(SERVO_ROT_CLOCK);
    }
    else if (direction == "stop")
    {
      Serial.println("stopping");
      rotState = STOP;
      myServo.write(SERVO_STOP);
    }
    else
    {
      direction = "Invalid Direction";
    }
  }
  else
  {
    direction = "Invalid Direction";
  }
  request->send(200, "text/plain", "Hello, POST: " + direction);
}

/**
 * @brief <IP>/stop route handler
 * 
 * Send a POST request to <IP>/stop with a form field message set to direction
 */
void handleStopRequest(AsyncWebServerRequest *request)
{
  String direction;

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
    if (direction == "stop")
    {
      Serial.println("stopping");
      rotState = STOP;
      myServo.write(SERVO_STOP);
    }
    else
    {
      direction = "Invalid Direction";
    }
  }
  else
  {
    direction = "Invalid Direction";
  }
  request->send(200, "text/plain", "Hello, POST: " + direction);
}

/**
 * @brief <IP>/resetPosition route handler
 * 
 * Send a POST request to <IP>/resetPosition to re-define the initial 0-offest
 */
void handleResetRequest(AsyncWebServerRequest *request)
{
  Serial.println("position reset");
  resetPosition();
  request->send(200, "text/plain", "Position Reset");
}

void handleGetStatusRequest(AsyncWebServerRequest *request)
{
  String status = (state == WIN_OPEN) ? "open" : "closed";
  request->send(200, "application/json", "{\"status\": \"" + status + "\"}");
}

/**
 * @brief tony from the bronx
 */
void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  WiFi.setAutoReconnect(true);

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}