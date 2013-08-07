#include <stdint.h>
#include "../libraries/TypeDefs.h"
#include "../libraries/RunningAvg.h"
#include "../libraries/HCTL2032SC.h"
#include "../libraries/PID.h"
#include "../trigint/Source/trigint.h"

// Define the pins to use with the counter
HCTL2032SC::pinList encPins;

//serial output period in microsconds
const uint16_t period = 1e6;
uint32_t time;
uint32_t lastTime;

uint8_t mtrPin;
uint8_t mtrIn1;
uint8_t mtrIn2;
uint8_t pwr;

void setup()
{	
    //setup motor
    mtrPin = 5;
    mtrIn1 = 7;
    mtrIn2 = 8;
    pinMode(mtrPin, OUTPUT);
    pinMode(mtrIn1, OUTPUT);
    pinMode(mtrIn2, OUTPUT);
    digitalWrite(mtrIn1, HIGH);
    digitalWrite(mtrIn2, LOW);
    pwr = 0;

    //set up serial port
    time = 0;
	lastTime = 0;
    Serial.begin(115200);
	delay(1000);
}

void loop()
{	
    time = micros();
	if((time - lastTime) > period)
    {
        pwr += 10;
        analogWrite(mtrPin,pwr);
        Serial.println(pwr);
        lastTime = time;
    }
}
