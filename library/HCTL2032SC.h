//subclass to get angle measurement from hctl-2032-sc encoder counter

#ifndef HCTL_2032_SC_H
#define HCTL_2032_SC_H

#include <stdint.h>
#include <arduino.h>
#include "encoder.h"
#include "pinStructs.h"

class HCTL2032SC: public Encoder
{
    private:
        const struct HCTL2032SCPinList pins;
        const uint8_t xORy;    //0 = x-axis, 1 = y-axis
        uint32_t resultLo;
        uint32_t result3rd;
        uint32_t result2nd;
        uint32_t resultHi;
        int32_t  result;

        uint8_t  GetByte();
    public:
        HCTL2032SC(const struct HCTL2032SCPinList &PinList, const uint8_t &whichAxis,
            Filter<int32_t> *filter);

        //sample encoder and compute angle in milliradians, update angleNew, and return value
        virtual int32_t MeasureAngle();
};

HCTL2032SC::HCTL2032SC(const struct HCTL2032SCPinList &PinList, const uint8_t &whichAxis,
    Filter<int32_t> *filter)
    : pins(PinList), xORy(whichAxis), Encoder(filter),
        resultLo(0), result3rd(0), result2nd(0), resultHi(0), result(0)
{
    //Configure Encoder Counter
    // Define port as input
    *(pins.DDRREG) = B00000000;

    //set count mode to 4x
    pinMode(pins.EN1,OUTPUT);
    digitalWrite(pins.EN1,HIGH);
    pinMode(pins.EN2,OUTPUT);
    digitalWrite(pins.EN2,LOW);
    
    // Select x OR y channel
    pinMode(pins.XY,OUTPUT);
    digitalWrite(pins.XY,xORy);

    // Reset by cycling RST to low then back to high
    pinMode(pins.RST,OUTPUT);
    digitalWrite(pins.RST,LOW);
    delay(10);
    digitalWrite(pins.RST,HIGH);

    // make sure read mdoe is off to begin with
    pinMode(pins.OE,OUTPUT);
    digitalWrite(pins.OE,HIGH);
    pinMode(pins.SEL1,OUTPUT);
    pinMode(pins.SEL2,OUTPUT);   
}
        
//sample encoder and compute angle in milliradians, update angleNew, and return value
int32_t HCTL2032SC::MeasureAngle()
{
    //choose X or Y axis
	digitalWrite(pins.XY,xORy);

	// get MSB
	digitalWrite(pins.SEL1,LOW);
	digitalWrite(pins.SEL2,HIGH);
	digitalWrite(pins.OE,LOW);    // Turn on read mode
	resultHi = ((uint32_t) GetByte()) & 0x000000FF;

	// Get 2nd Byte
	digitalWrite(pins.SEL1,HIGH);
	digitalWrite(pins.SEL2,HIGH);
	result2nd = ((uint32_t) GetByte()) & 0x000000FF;
	
	// Get 3rd Byte
	digitalWrite(pins.SEL1,LOW);
	digitalWrite(pins.SEL2,LOW);
	result3rd = ((uint32_t) GetByte()) & 0x000000FF;
	
	// Get LSB
	digitalWrite(pins.SEL1,HIGH);
	digitalWrite(pins.SEL2,LOW);
	resultLo = ((uint32_t) GetByte()) & 0x000000FF;
	
    // Turn off read mode
	digitalWrite(pins.OE,HIGH);
	
	//Combine Bytes to get 32 bit encoder count
	result = (int32_t) ((resultHi << 24) + (result2nd << 16) + 
		(result3rd << 8) + resultLo);

    //convert encoder ticks to milliradians, update angleNew, and return value
    const int16_t twoPIx1000 = 6283;    //two pi times 1000 to return angle in milliradians
    const int16_t countsPerRev = 1440;  //number of encoder ticks per revolution
    angleNew = result*twoPIx1000/countsPerRev;
    return angleNew;
}

uint8_t  HCTL2032SC::GetByte()
{
    uint8_t byteOld;
	uint8_t byteNew;

    byteOld = *(pins.PINREG);
    byteNew = *(pins.PINREG);

    if(byteOld == byteNew)
	{	
		return byteNew;
	}
	else
	{   
		return HCTL2032SC::GetByte();
	}
}
#endif
