#include <stdint.h>
#include "../libraries/TypeDefs.h"
#include "../libraries/RunningAvg.h"
#include "../libraries/HCTL2032SC.h"
#include "../libraries/pinStructs.h"
#include "../libraries/PID.h"
#include "../trigint/Source/trigint.h"

// Define the pins to use with the counter
HCTL2032SCPinList encPins;

//serial output period in microsconds
const uint16_t serialPer = 2e4;
uint32_t time;
uint32_t serialLastTime;

//declare encoder counter and its filter
const uint8_t windowSize = 10; //in milliseconds
const uint8_t axis = 1;
const int8_t direction = 1;
RunningAvg<int_vel_t> *speedFilter = 0; 
HCTL2032SC *encoder1 = 0;

//define motor PID gains
//Numerator of gain is Knx
//Denominator of gain is 2^(Kdx)
const uint16_t Knp = 40;
const uint16_t Kni = 20;
const uint16_t Knd = 0;

const uint8_t Kdp = 4;
const uint8_t Kdi = 12;
const uint8_t Kdd = 0;

//declare controller and its filter
int_vel_t speedConv(uint8_t speedRad);
uint8_t feedForward(int_vel_t desSpeedRad);

uint8_t desSpeedRad = 0;
int_vel_t desSpeed =  0;
RunningAvg<int_accel_t> *accelFilter = 0; 
PID *PIDcntrl = 0;

uint8_t mtrPin;
int16_t pwrPID;
int16_t pwrFF;
int16_t pwr;

int_vel_t speed;
trigint_angle_t angle;
void setup()
{	
    //setup timer1 for control loop, prescale = 1
    const uint16_t cntrlFrq = 1000; 
    const uint32_t clkFrq = 16e6; 
    const uint16_t count = clkFrq/cntrlFrq;
    cli();						// disable global interrupts
    TCCR1A = 0;					// set entire TCCR0A register to 0
    TCCR1B = 0;					// set entire TCCR0B register to 0
    TCNT1 = 0;					// set initial count to 0
    OCR1A = count;
    TCCR1B |= (1 << WGM12);		// turn on CTC mode
    TCCR1B |= (1 << CS10);      // prescale = 1
    TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
			
    //assign encoder pins and registers and create encoder object
    encPins.SEL1 = 36;
    encPins.SEL2 = 34;
    encPins.EN1 = 33;
    encPins.EN2 = 32;
    encPins.OE = 37;
    encPins.RST = 30;
    encPins.XY = 31;
    encPins.PINREG = &PINA;
    encPins.DDRREG = &DDRA;

    speedFilter = new RunningAvg<int_vel_t>(windowSize);
    encoder1 = new HCTL2032SC(encPins,axis,direction,speedFilter);
    
    //create controller object
    accelFilter = new RunningAvg<int_accel_t>(windowSize);
    PIDcntrl = new PID(Knp,Kni,Knd,Kdp,Kdi,Kdd,0,desSpeed,accelFilter);
    
    mtrPin = 10;
    pinMode(mtrPin, OUTPUT);
    pwr = 0;
    pwrPID = 0;
    pwrFF = 0;
    sei();						// enable global interrupts

    //set up serial port
    time = 0;
	serialLastTime = 0;
    Serial.begin(115200);
	delay(1000);
}

ISR(TIMER1_COMPA_vect)
{
    encoder1->MeasureSpeed();
    /*
    if(encoder1->GetAngle() < 0x2000)
    {
        desSpeedRad = 40;
        pwrFF = feedForward(desSpeedRad);
        desSpeed = speedConv(desSpeedRad);
    }
    else
    {
        desSpeedRad = 40;
        pwrFF = feedForward(desSpeedRad);
        desSpeed = speedConv(desSpeedRad);
    }
    */
    pwrPID = PIDcntrl->Calculate(desSpeed,encoder1->GetSpeed(),pwr-pwrFF);
    pwr = pwrFF + pwrPID;
    if (pwr > 255)
    {
        pwr = 255;
    }
    else if (pwr < 0)
    {
        pwr = 0;
    }
    analogWrite(mtrPin,pwr);
}

void loop()
{	
    time = micros();
	if((time - serialLastTime) > serialPer)
    {
        cli();
        speed = encoder1->GetSpeed();
        angle = encoder1->GetAngle();
        uint8_t pwr2 = pwr;
        uint8_t pwrFF2 = pwrFF;
        uint8_t pwrPID2 = pwrPID;
        sei();
        if ((time > 3e6) && (desSpeedRad != 30))
        {
            desSpeedRad = 30;
            int_vel_t temp = speedConv(desSpeedRad);
            cli();
            desSpeed = temp;
            
            sei();
            pwrFF = feedForward(desSpeedRad);
        }
        Serial.print(time);
		Serial.print("  ");
        Serial.print(speed);
		Serial.print("  ");
        Serial.print(pwr2);
		Serial.print("  ");
        Serial.print(pwrFF2);
		Serial.print("  ");
        Serial.println(pwrPID2);
        //uint8_t checksum = static_cast<uint8_t>(time + speed);
		//Serial.print("  ");
        //Serial.println(checksum);

        serialLastTime = time;
	}
}


int_vel_t speedConv(uint8_t speedRad)
{
    const int32_t rad_s2cnt_1024us= 2670177; //trig int angles per cycle * 1024/2*pi
    return speedRad*rad_s2cnt_1024us/1e6;
}

uint8_t feedForward(int_vel_t desSpeedRad)
{
    return desSpeedRad * 4;
}

