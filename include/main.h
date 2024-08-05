#define SLOW 0
#define NORMAL 1
#define FAST 2
#define SPEED_MODE NORMAL

#if SPEED_MODE == 0
    #define SERVO_ROT_CLOCK 88
    #define SERVO_ROT_COUNTER 100
    #define SERVO_STOP 92
    #define MARGIN 5
#elif SPEED_MODE == 1
    #define SERVO_ROT_CLOCK 80
    #define SERVO_ROT_COUNTER 108
    #define SERVO_STOP 92
    #define MARGIN 10
#elif SPEED_MODE == 2
    #define SERVO_ROT_CLOCK 0 
    #define SERVO_ROT_COUNTER 180
    #define SERVO_STOP 92
    #define MARGIN 30
#else
    #error "Invalid SPEED_MODE defined"
#endif

// pin defintions
const uint8_t servoPin = 4;
const uint8_t btn1Pin = 14;
const uint8_t btn2Pin = 27;
const uint8_t ledBuiltinPin = 2;

// function prototypes
void notFound(AsyncWebServerRequest *request);
void handleCurtainRequest(AsyncWebServerRequest *request);
void handleRotateRequest(AsyncWebServerRequest *request);
void handleStopRequest(AsyncWebServerRequest *request);
void handleResetRequest(AsyncWebServerRequest *request);
void handleGetStatusRequest(AsyncWebServerRequest *request);
void wifiSetup();
