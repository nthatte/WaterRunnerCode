#include <stdint.h>
#include "../libraries/RunningAvg.h"
#include "../libraries/HCTL2032SC.h"
#include "../libraries/pinStructs.h"
#include "../trigint/Source/trigint.h"

// Define the pins to use with the counter
HCTL2032SCPinList encPins;

//serial output period in milliseconds
const uint16_t serialPer = 100;
uint32_t time;
uint32_t serialLastTime;

//create filter and encoder counter objects
const uint8_t windowSize = 1000;
const uint8_t axis = 1;
const int8_t direction = 1;
RunningAvg<int32_t> *speedFilter = 0; 
HCTL2032SC *encoder1 = 0;

uint8_t mtrPin;
uint8_t pwr;

int32_t speed;
trigint_angle_t angle;

void setup()
{	
    //setup timer1 for control loop, prescale = 1
    const uint16_t cntrlFrq = 1000; 
    const uint32_t clkFrq = 16e6; 
    const uint16_t count = static_cast<const uint16_t>(clkFrq/cntrlFrq);
    cli();						// disable global interrupts
    TCCR1A = 0;					// set entire TCCR0A register to 0
    TCCR1B = 0;					// set entire TCCR0B register to 0
    TCNT1 = 0;					// set initial count to 0
    OCR1A = count;
    TCCR1B |= (1 << WGM12);		// turn on CTC mode
    TCCR1B |= (1 << CS10);      // prescale = 1
    TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
    sei();						// enable global interrupts
			
    //assign encoder pins and registers
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
    encoder1 = new HCTL2032SC(encPins,axis,direction,speedFilter);

    //set up serial port
    time = 0;
	serialLastTime = 0;
    Serial.begin(115200);
	delay(1000);
    Serial.println("here2");

    mtrPin = 10;
    pinMode(mtrPin, OUTPUT);
}

ISR(TIMER1_COMPA_vect)
{
    encoder1->MeasureSpeed();
}

void loop()
{	
    time = millis();
	if((time - serialLastTime) > serialPer){
        cli();
        speed = encoder1->GetSpeed();
        anlge = encoder1->GetAngle();
        sei();
        Serial.print(time);
		Serial.print("  ");
        Serial.print(speed);
		Serial.print("  ");
		Serial.println(angle);

		serialLastTime = time;
	}
}
