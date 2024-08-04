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
enum windowState {
  WIN_CLOSED = 0,
  WIN_OPEN,
  WIN_TRANSITION_CLOSE,
  WIN_TRANSITION_OPEN
};
// open state could potentially be updated when we are between the marging and edge of max rotation interval.
windowState state = WIN_CLOSED;
enum rotationState {
  STOP = 0,
  ROT_LEFT,
  ROT_RIGHT
};
rotationState rotState = STOP;

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
  globalAngle = updateRotation();

  if (button1.pressed())
  {
    rotState = ROT_LEFT;
    myServo.write(SERVO_ROT_CLOCK);
    Serial.println("rotleft");
  }
  else if (button1.released())
  {
    rotState = STOP;
    myServo.write(SERVO_STOP);
    Serial.println("release");
  }

  if (button2.pressed())
  {
    rotState = ROT_RIGHT;
    myServo.write(SERVO_ROT_COUNTER);
    Serial.println("rotright");
  }

  else if (button2.released())
  {
    rotState = STOP;
    myServo.write(SERVO_STOP);
    Serial.println("release");
  }

  if (state == WIN_TRANSITION_CLOSE)
  {
    if (flag)
    {
      // clock
      myServo.write(SERVO_ROT_CLOCK);
      flag = false;
    }

    if (globalAngle >= maxRotationInterval[1] - MARGIN)
    {
      myServo.write(SERVO_STOP);
      state = WIN_CLOSED;
      flag = true;
    }
  }
  else if (state == WIN_TRANSITION_OPEN)
  {
    if (flag)
    {
      // counter
      myServo.write(SERVO_ROT_COUNTER);
      flag = false;
    }

    if (globalAngle <= maxRotationInterval[0] + MARGIN)
    {
      myServo.write(SERVO_STOP);
      state = WIN_OPEN;
      flag = true;
    }
  }
  else if (rotState)
  {
    if (globalAngle >= maxRotationInterval[1] - MARGIN && rotState == ROT_LEFT)
    {
      myServo.write(SERVO_STOP);
      rotState = STOP;
    }
    else if (globalAngle <= maxRotationInterval[0] + MARGIN && rotState == ROT_RIGHT)
    {
      myServo.write(SERVO_STOP);
      rotState = STOP;
    }
  }
  digitalWrite(ledBuiltinPin, WiFi.isConnected());
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