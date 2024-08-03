#define SPEED_MODE NORMAL

#if SPEED_MODE == SLOW
    #define SERVO_ROT_CLOCK 88
    #define SERVO_ROT_COUNTER 100
    #define SERVO_STOP 92
#elif SPEED_MODE == NORMAL
    #define SERVO_ROT_CLOCK 80
    #define SERVO_ROT_COUNTER 108
    #define SERVO_STOP 92
#elif SPEED_MODE == FAST
    #define SERVO_ROT_CLOCK 0 
    #define SERVO_ROT_COUNTER 180
    #define SERVO_STOP 92
#else
    #error "Invalid SPEED_MODE defined"
#endif