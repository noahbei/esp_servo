#include <stddef.h>

#define GENERAL_ANGLE 1

#define AS5600_ADDR 0x36
#define OLED_ADDR 0x3C
#define RST_PIN -1

#define SDA_PIN 21
#define SCL_PIN 18

// helper functions
void ReadRawAngle();
void correctAngle();
void checkQuadrant();
void refreshDisplay();
void resetPosition();

// main driving functions
void setupEncoder();
float updateRotation();