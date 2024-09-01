#include <Arduino.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>
#include <Button.h>
#include <string>
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
windowTransitionState prevState = WIN_STOP;

// does this define an end state variable?
enum windowEndState {
  OPEN = 0,
  CLOSE,
  MIDDLE
} endState;
// open state could potentially be updated when we are between the marging and edge of max rotation interval.

float globalAngle = 0;
const int maxRotationInterval[] = {0, 900}; // max rotation range in degrees

struct Schedule {
    std::string hour;
    std::string minute;
    std::string tod; // time of day (am/pm)
};
Schedule openTime = {"10", "00", "am"};
Schedule closeTime = {"09", "00", "pm"};

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
  server.on("/schedule", HTTP_POST, handleSchedulePost);
  server.on("/schedule", HTTP_GET, handleScheduleGet);
  server.on("/state", HTTP_GET, handleStateGet);

  server.onNotFound(notFound);
  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
}

unsigned long previousMillis = 0;
const long interval = 10000; // 10 seconds
void loop()
{
  ElegantOTA.loop();
  globalAngle = updateRotation();

  // lock control to rotate open/closed when win_transition_x is active
  if (!(rotationState == WIN_TRANSITION_OPEN || rotationState == WIN_TRANSITION_CLOSE)) {
    if (button1.pressed()) rotationState = WIN_ROTATE_OPEN;
    else if (button2.pressed()) rotationState = WIN_ROTATE_CLOSE;
    else if (button1.released() || button2.released()) rotationState = WIN_STOP;
  }
  else {

  }
  // make sure window can't be rotated out of bounds
  if ((globalAngle >= maxRotationInterval[1] - MARGIN && (rotationState == WIN_ROTATE_OPEN || rotationState == WIN_TRANSITION_OPEN)) ||
      (globalAngle <= maxRotationInterval[0] + MARGIN && (rotationState == WIN_ROTATE_CLOSE || rotationState == WIN_TRANSITION_CLOSE)))
  {
    rotationState = WIN_STOP;
  }

  if (prevState != rotationState) {
    switch (rotationState) {
      case WIN_TRANSITION_OPEN:
      case WIN_ROTATE_OPEN:
        myServo.write(SERVO_ROT_CLOCK);
        Serial.println("rotating open");
        break;
      case WIN_TRANSITION_CLOSE:
      case WIN_ROTATE_CLOSE:
        myServo.write(SERVO_ROT_COUNTER);
        Serial.println("rotating close");
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
  }
  prevState = rotationState;

  // check wifi status every 10 seconds
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    digitalWrite(ledBuiltinPin, WiFi.isConnected());
  }
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
  if (rotationState == WIN_TRANSITION_CLOSE || rotationState == WIN_TRANSITION_OPEN)
  {
    request->send(403, "text/plain", "Cannot control encoder, current state: " + String(rotationState));
    return;
  }

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
    if (direction == "open")
    {
      rotationState = WIN_TRANSITION_OPEN;
    }
    else if (direction == "close")
    {
      rotationState = WIN_TRANSITION_CLOSE;
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
  if (rotationState == WIN_TRANSITION_CLOSE || rotationState == WIN_TRANSITION_OPEN)
  {
    request->send(403, "text/plain", "Cannot control encoder, current state: " + String(rotationState));
    return;
  }

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
    if (direction == "clockwise")
    {
      rotationState = WIN_ROTATE_CLOSE;
    }
    else if (direction == "counterclockwise")
    {
      rotationState = WIN_ROTATE_OPEN;
    }
    else if (direction == "stop")
    {
      rotationState = WIN_STOP;
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
      rotationState = WIN_STOP;
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
 * @brief <IP>/schedule route handler
 * 
 * Send a POST request to <IP>/schedule with a form field message set to open and close
 */
void handleSchedulePost(AsyncWebServerRequest *request)
{
  //param will be schedule json, will have to update the schedule object using this
  String direction;
  if (rotationState == WIN_TRANSITION_CLOSE || rotationState == WIN_TRANSITION_OPEN)
  {
    request->send(403, "text/plain", "Cannot control encoder, current state: " + String(rotationState));
    return;
  }

  if (request->hasParam(PARAM_DIRECTION, true))
  {
    direction = request->getParam(PARAM_DIRECTION, true)->value();
  }
  else
  {
    direction = "Invalid Direction";
  }
  request->send(200, "text/plain", "Hello, POST: " + direction);
}

/**
 * @brief <IP>/schedule route handler
 * 
 * Send a GET request to <IP>/schedule to get current open and close times
 */
void handleScheduleGet(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "openTime");
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
  String status = (endState == OPEN) ? "open" : "closed";
  request->send(200, "application/json", "{\"status\": \"" + status + "\"}");
}

void handleStateGet(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", String(rotationState));
}

/**
 * @brief tony from the bronx
 */
void wifiSetup()
{
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

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}