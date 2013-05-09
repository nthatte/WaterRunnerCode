#include <stdint.h>
#include "../library/RunningAvg.h"
#include "../library/HCTL2032SC.h"
#include "../library/pinStructs.h"

// Define the pins to use with the counter
HCTL2032SCPinList encPins;

//serial output period in microseconds
const uint16_t serialPer = 100;
uint32_t time;
uint32_t serialLastTime;
int32_t speed;
int32_t angle;

//create filter and encoder counter objects
const uint8_t windowSize = 1000;
const uint8_t axis = 1;
RunningAvg<int32_t> *speedFilter = 0; 
HCTL2032SC *encoder1 = 0;

void setup()
{	
    //assign encoder pin
    encPins.SEL1 = 36;
    encPins.SEL2 = 34;
    encPins.EN1 = 33;
    encPins.EN2 = 32;
    encPins.OE = 37;
    encPins.RST = 30;
    encPins.XY = 31;
    encPins.PINREG = &PINA;
    encPins.DDRREG = &DDRA;

    speedFilter = new RunningAvg<int32_t>(windowSize);
    encoder1 = new HCTL2032SC(encPins,axis,speedFilter);

    //set up serial port
    time = 0;
	serialLastTime = 0;
    Serial.begin(115200);
	delay(1000);
    Serial.println("here2");

}

void loop()
{	
    time = millis();
    speed = encoder1->MeasureSpeed();
	if((time - serialLastTime) > serialPer){
        Serial.print(time);
		Serial.print("  ");
		Serial.println(speed);

		serialLastTime = time;
	}
}
