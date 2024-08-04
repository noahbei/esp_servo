#define SLOW 0
#define NORMAL 1
#define FAST 2
#define SPEED_MODE FAST

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