#include <stddef.h>
#define AS5600_ADDR 0x36

#define SDA_PIN 21
#define SCL_PIN 18

// helper functions
void ReadRawAngle();
void correctAngle();
void checkQuadrant();
void refreshDisplay();

// main driving functions
void setupEncoder();
float updateRotation();