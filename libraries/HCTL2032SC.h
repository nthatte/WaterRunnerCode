/**
 *  Class to setup pins and get angle measurments from
 *  HCTL-2032-SC encoder counter 
 */

#ifndef HCTL_2032_SC_H
#define HCTL_2032_SC_H

#include <stdint.h>
#include <arduino.h>
#include "encoder.h"

class HCTL2032SC: public Encoder
{
    public:
        struct pinList
        {
            uint8_t SEL1;
            uint8_t SEL2;
            uint8_t EN1;
            uint8_t EN2;
            uint8_t OE;
            uint8_t RST;
            uint8_t XY;
            volatile uint8_t *PINREG;
            volatile uint8_t *DDRREG;
        };

        HCTL2032SC(const pinList &PinList, const uint8_t &whichAxis,
            const int8_t &cDirection, Filter<int16_t> *filter);

        virtual trigint_angle_t MeasureAngle();

    private:
        const pinList pins;
        const uint8_t xORy;      //0 = x-axis, 1 = y-axis
        const int8_t direction;  //1 = forward, -1 = backwards
        uint32_t resultLo;
        uint32_t result3rd;
        uint32_t result2nd;
        uint32_t resultHi;
        int32_t  result;
        uint32_t resultSat;
        
        inline uint8_t  GetByte();
};

/**
 *  Constructs new encoder counter objects. 
 *  Sets pins from pin list struct
 */
HCTL2032SC::HCTL2032SC(const pinList &PinList, const uint8_t &whichAxis,
    const int8_t &cDirection, Filter<int16_t> *filter)
    : pins(PinList), xORy(whichAxis), direction(cDirection), Encoder(filter),
        resultLo(0), result3rd(0), result2nd(0), resultHi(0), result(0)
{
    // Configure Encoder Counter
    // Define port as input
    *(pins.DDRREG) = B00000000;

    // set count mode to 4x
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
        
/**
 *  sample encoder and compute angle in milliradians, update angleNew, and return value
 */
trigint_angle_t HCTL2032SC::MeasureAngle()
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
	result = direction*((int32_t) ((resultHi << 24) + (result2nd << 16) + 
		(result3rd << 8) + resultLo));

    /*
    saturate result so it is between 0 and countsPerRev and positive 
    otherwise result can be small negative or very large
    and would cause over flow when converted to angle units 
    */
    const int16_t countsPerRev = 1440;   //number of encoder ticks per revolution
    resultSat = (result<0) ? 0 : result%countsPerRev;
    angleNew = (resultSat*TRIGINT_ANGLES_PER_CYCLE)/countsPerRev; 
    return angleNew;
}

inline uint8_t  HCTL2032SC::GetByte()
{
    uint8_t byteOld;
	uint8_t byteNew;
    
    //Request byte until stable
    do{
        byteOld = *(pins.PINREG);
        byteNew = *(pins.PINREG);
    }while (byteOld != byteNew);
    	
    return byteNew;
}
#endif
