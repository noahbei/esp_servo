#include "MagneticEncoder.h"
#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>

void rotateServo();
int mapValueToRange(int userSpeed, int range[2]);
void rotateServo(int speed, int degrees);

//Servo myServo;
//const uint8_t servoPin = 4;
// ranges for servo motor
struct Range {
    int clockwise[2];
    int stop[2];
    int counter[2];
};
Range ranges = {
    // left val is slow, right val is fast
    {91, 0},
    {92, 96},
    {97, 180}
};

float OLEDTimer = 0;

int lowbyte; //raw angle 7:0
word highbyte; //raw angle 7:0 and 11:8
int rawAngle; //final raw angle 
float degAngle; //raw angle in degrees (360/4096 * [value between 0-4095])

int quadrantNumber, previousquadrantNumber; //quadrant IDs
float numberofTurns = 0; //number of turns
float correctedAngle = 0; //tared angle - based on the startup value
float startAngle = 0; //starting angle
float totalAngle = 0; //total absolute angular displacement
float previoustotalAngle = 0; //for the display printing
const int* maxRotationInterval = nullptr;
size_t maxRotationIntervalSize = 0;

void setupEncoder()
{
  //Serial.begin(115200); //start serial - tip: don't use serial if you don't need it (speed considerations)
  Wire.begin(SDA_PIN, SCL_PIN); //start i2C  
  Wire.setClock(800000L); //fast clock

  // here I would get the value from eeprom which would be configured in the initial calibration mode which will be set to not calibrated.
  // update calibrated bit in eeprom?
  ReadRawAngle(); //make a reading so the degAngle gets updated
  startAngle = degAngle; //update startAngle with degAngle - for taring

  Serial.println("Welcome!"); //print a welcome message  
  Serial.println("AS5600"); //print a welcome message
  delay(3000);
  OLEDTimer = millis(); //start the timer
}

float updateRotation() {
    ReadRawAngle(); //ask the value from the sensor
    correctAngle(); //tare the value
    checkQuadrant(); //check quadrant, check rotations, check absolute angular position
    //refreshDisplay();

    //delay(100); //wait a little - adjust it for "better resolution"
    //100 is the degrees that we set when we start the rotation
    // if (totalAngle < maxRotationInterval[0] || totalAngle > maxRotationInterval[1]) {
    //     myServo.write(ranges.stop[0]);
    //     return 1;
    // }
    // if (totalAngle > deg) {
    //     myServo.write(ranges.stop[0]);
    //     return 1;
    // }
    //delay(100);
    Serial.print("angle: ");
    Serial.println(totalAngle);
    return totalAngle;
}

void ReadRawAngle()
{ 
  //7:0 - bits
  Wire.beginTransmission(AS5600_ADDR); //connect to the sensor
  Wire.write(0x0D); //figure 21 - register map: Raw angle (7:0)
  Wire.endTransmission(); //end transmission
  Wire.requestFrom(AS5600_ADDR, 1); //request from the sensor
  
  while(Wire.available() == 0); //wait until it becomes available 
  lowbyte = Wire.read(); //Reading the data after the request
 
  //11:8 - 4 bits
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C); //figure 21 - register map: Raw angle (11:8)
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 1);
  
  while(Wire.available() == 0);  
  highbyte = Wire.read();
  
  //4 bits have to be shifted to its proper place as we want to build a 12-bit number
  highbyte = highbyte << 8; //shifting to left
  //What is happening here is the following: The variable is being shifted by 8 bits to the left:
  //Initial value: 00000000|00001111 (word = 16 bits or 2 bytes)
  //Left shifting by eight bits: 00001111|00000000 so, the high byte is filled in
  
  //Finally, we combine (bitwise OR) the two numbers:
  //High: 00001111|00000000
  //Low:  00000000|00001111
  //      -----------------
  //H|L:  00001111|00001111
  rawAngle = highbyte | lowbyte; //int is 16 bits (as well as the word)

  //We need to calculate the angle:
  //12 bit -> 4096 different levels: 360° is divided into 4096 equal parts:
  //360/4096 = 0.087890625
  //Multiply the output of the encoder with 0.087890625
  degAngle = rawAngle * 0.087890625; 
  
  //Serial.print("Deg angle: ");
  //Serial.println(degAngle, 2); //absolute position of the encoder within the 0-360 circle
  
}

void correctAngle()
{
  //recalculate angle
  correctedAngle = degAngle - startAngle; //this tares the position

  if(correctedAngle < 0) //if the calculated angle is negative, we need to "normalize" it
  {
  correctedAngle = correctedAngle + 360; //correction for negative numbers (i.e. -15 becomes +345)
  }
  else
  {
    //do nothing
  }
  //Serial.print("Corrected angle: ");
  //Serial.println(correctedAngle, 2); //print the corrected/tared angle  
}

void checkQuadrant()
{
  /*
  //Quadrants:
  4  |  1
  ---|---
  3  |  2
  */

  //Quadrant 1
  if(correctedAngle >= 0 && correctedAngle <=90)
  {
    quadrantNumber = 1;
  }

  //Quadrant 2
  if(correctedAngle > 90 && correctedAngle <=180)
  {
    quadrantNumber = 2;
  }

  //Quadrant 3
  if(correctedAngle > 180 && correctedAngle <=270)
  {
    quadrantNumber = 3;
  }

  //Quadrant 4
  if(correctedAngle > 270 && correctedAngle <360)
  {
    quadrantNumber = 4;
  }
  //Serial.print("Quadrant: ");
  //Serial.println(quadrantNumber); //print our position "quadrant-wise"

  if(quadrantNumber != previousquadrantNumber) //if we changed quadrant
  {
    if(quadrantNumber == 1 && previousquadrantNumber == 4)
    {
      numberofTurns++; // 4 --> 1 transition: CW rotation
    }

    if(quadrantNumber == 4 && previousquadrantNumber == 1)
    {
      numberofTurns--; // 1 --> 4 transition: CCW rotation
    }
    //this could be done between every quadrants so one can count every 1/4th of transition

    previousquadrantNumber = quadrantNumber;  //update to the current quadrant
  
  }  
  //Serial.print("Turns: ");
  //Serial.println(numberofTurns,0); //number of turns in absolute terms (can be negative which indicates CCW turns)  

  //after we have the corrected angle and the turns, we can calculate the total absolute position
  totalAngle = (numberofTurns*360) + correctedAngle; //number of turns (+/-) plus the actual angle within the 0-360 range
  //Serial.print("Total angle: ");
  //Serial.println(totalAngle, 2); //absolute position of the motor expressed in degree angles, 2 digits
}

void refreshDisplay()
{
  if (millis() - OLEDTimer > 100) //chech if we will update at every 100 ms
	{ 
    if(totalAngle != previoustotalAngle) //if there's a change in the position*
    {
        
        Serial.println(totalAngle); //print the new absolute position
        OLEDTimer = millis(); //reset timer 	
        previoustotalAngle = totalAngle; //update the previous value
    }
	}
	else
	{
		//skip
	}
  //*idea: you can define a certain tolerance for the angle so the screen will not flicker
  //when there is a 0.08 change in the angle (sometimes the sensor reads uncertain values)
}

// /**
//  * @brief rotates the servo
//  * 
//  * Detailed explanation of what the function does. You can explain the 
//  * overall behavior, any side effects, or important details.
//  * 
//  * @param speed speed should be between 1 - 50 (may change range if some low values don't move servo)
//  * @param direction direction should be between
//  * @return 1 if this returns ok, 0 if error occurred
//  */
// void rotateServo(int speed) {

//   myServo.write(speed);
// }

// void rotateServo(int speed, int degrees) {
//   bool direction = degrees > totalAngle;
//   speed = mapValueToRange(speed, direction ? ranges.clockwise : ranges.counter);
//   myServo.write(speed);
//   if (totalAngle > degrees) {
//     myServo.write(ranges.stop[0]);
//   }
// }

// int mapValueToRange(int userSpeed, int range[2]) {
//     int minInput = 1;
//     int maxInput = 50;

//     int minRange = range[0];
//     int maxRange = range[1];

//     // Ensure userSpeed is within the input range
//     if (userSpeed < minInput) userSpeed = minInput;
//     if (userSpeed > maxInput) userSpeed = maxInput;
    
//     // Linear mapping formula
//     return (int)(((float)(userSpeed - minInput) / (maxInput - minInput)) * (maxRange - minRange) + minRange);
// }

// void ServoWrite(int degree) {
//     myServo.write(degree);
// }