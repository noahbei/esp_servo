#define AS5600_ADDR 0x36  // Default I2C address for AS5600

#define sdaPin 21
#define sclPin 18

void ReadRawAngle();
void correctAngle();
void checkQuadrant();
void refreshDisplay();

void setupServo(const int* interval, size_t size);
void rotateServo(int speed, int degrees);
int updateRotation(float deg);