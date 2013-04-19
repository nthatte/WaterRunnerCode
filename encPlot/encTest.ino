#include <stdint.h>
#include <math.h>
#include "../library/RunningAvg.h"

// Define the pins to use with the counter
#define SEL1 36
#define SEL2 34
#define EN1 33
#define EN2 32
#define OE 37
#define RSTX 35
#define RSTY 30
#define X_Y 31

//serial output period in microseconds
#define SERIAL_PER 1e5 

#define AVG_WINDOW 100

//Encoder counter results
uint32_t Result_lo;
uint32_t Result_3rd;
uint32_t Result_2nd;
uint32_t Result_hi;
int32_t Result;
double Angle1;
double Angle2;
double Angle_deg;
double Time0;
double Time1;
double Time2;
double Speed1;
double Speed2;
RunningAvg<double> SpeedAvg(AVG_WINDOW);

double serialLastTime;

//constants and variables for error correction
#define ERR_DV_THRESH 20
double corrFactor;
double Angle_pred;

// for incoming serial data
uint16_t incomingByte = 0;
uint16_t oldByte = 0;

//forward declare functions
uint8_t Get_Byte();
double getAngleRad(int32_t count);
double rad2Deg(double angleRad);
double getSpeed(double angle1, double angle2, double time1, double time2);

void setup()
{	
	//Initialize Counters
	Result_lo = 0;
	Result_3rd = 0;
	Result_2nd = 0;
	Result_hi = 0;
	Result =0;
	Angle1 = 0;
	Angle2 = 0;
	Angle_deg = 0;
	Angle_pred = 0;
	Time0 = 0;
	Time1 = 0;
	Time2 = 0;
	Speed1 = 0;
	Speed2 = 0;
    corrFactor = 0;
	SpeedAvg.newReading(0);

	serialLastTime = 0;

    //Configure Encoder Counter
    // EN1=1
    pinMode(EN1,OUTPUT);
    digitalWrite(EN1,HIGH);
    // EN2=0
    pinMode(EN2,OUTPUT);
    digitalWrite(EN2,LOW);
    // Select Y channel
    pinMode(X_Y,OUTPUT);
    digitalWrite(X_Y,HIGH);

    // Reset, Rst_x Rst_y set to low since it is active low
    pinMode(RSTX,OUTPUT);
    pinMode(RSTY,OUTPUT);
    digitalWrite(RSTX,LOW);
    digitalWrite(RSTY,LOW);
    delay(100);
    digitalWrite(RSTX,HIGH);
    digitalWrite(RSTY,HIGH);
    // Disable OE
    pinMode(OE,OUTPUT);
    digitalWrite(OE,HIGH);
    pinMode(SEL1,OUTPUT);
    pinMode(SEL2,OUTPUT);
    
    // Define input pins
    DDRA = B00000000;

    Serial.begin(115200);
	delay(1000);
}


void loop()
{	
	delay(1);
	// Turn off read mode

	// SEL1 and SEL2 for MSB
	digitalWrite(SEL1,LOW);
	digitalWrite(SEL2,HIGH);
	// Turn on read mode
	digitalWrite(OE,LOW);
	// Get MSB
	Result_hi = ((uint32_t) Get_Byte()) & 0x000000FF;

	// SEL1 and SEL2 for 2nd BYTE
	digitalWrite(SEL1,HIGH);
	digitalWrite(SEL2,HIGH);
	// Get 2nd Byte
	Result_2nd = ((uint32_t) Get_Byte()) & 0x000000FF;
	
	// sSEL1 and SEL2 for 3rd BYTE
	digitalWrite(SEL1,LOW);
	digitalWrite(SEL2,LOW);
	// Get 3rd Byte
	Result_3rd = ((uint32_t) Get_Byte()) & 0x000000FF;
	
	// SEL1 and SEL2 for LSB
	digitalWrite(SEL1,HIGH);
	digitalWrite(SEL2,LOW);
	// Get LSB
	Result_lo = ((uint32_t) Get_Byte()) & 0x000000FF;
	
	digitalWrite(OE,HIGH);
	
	//Combine Bytes to get 32 bit encoder count		
	Result = (int32_t) ((Result_hi << 24) + (Result_2nd << 16) + 
		(Result_3rd << 8) + Result_lo);
	Angle2 = Angle1;
	Time2 = Time1;
	Speed2 = Speed1;

	Angle1 = getAngleRad(Result) + corrFactor;
	Time1 = (double) micros();
	Speed1 = getSpeed(Angle1,Angle2,Time1,Time2);
	
	//Correct erroneous readings when speed difference is too large 
	//by extrapolating forwards from last angle using last speed.
	if((Speed1 > SpeedAvg.getAvg() + ERR_DV_THRESH) || 
			(Speed1 < SpeedAvg.getAvg() - ERR_DV_THRESH )|| 
			(Speed1 < -1.) || 
			(Speed1 > 60.))
	{
		Speed1 = SpeedAvg.getAvg();
		Angle_pred = Angle2 + Speed1*(Time1 - Time2)/1e6;
		corrFactor += Angle_pred - Angle1;
		Angle1 = Angle_pred;
	}
	SpeedAvg.newReading(Speed1);

	if((Time1 - serialLastTime) > SERIAL_PER){
		Serial.print(Time1/1e6);
		Serial.print("  ");
		Serial.print(SpeedAvg.getAvg());
		Serial.print("  ");
		Serial.print(rad2Deg(Angle1));
		Serial.print("  ");
		Serial.println(corrFactor);
		serialLastTime = Time1;
	}
}

uint8_t Get_Byte()
{
	uint8_t Byte_old;
	uint8_t Byte_new;

    Byte_old = PINA;
    Byte_new = PINA;

    if(Byte_old == Byte_new)
	{	
		return Byte_new;
	}
	else
	{   
		return Get_Byte();
	}
}

double getAngleRad(int32_t count)
{
	return ((double) count) * 2*PI/1440.;
}

double rad2Deg(double angleRad)
{
	return fmod((angleRad * 180/3.14159),360.);
}

double getSpeed(double angle1, double angle2, double time1, double time2)
{
	return (angle1 - angle2)*1.e6/(time1 - time2);
}
