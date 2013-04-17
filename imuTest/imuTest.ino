#include <stdint.h>
#include <math.h>

// Define the pins to use with the counter
#define D0 22
#define D1 23
#define D2 24
#define D3 25
#define D4 26
#define D5 27
#define D6 28
#define D7 29
#define SEL1 36
#define SEL2 34
#define EN1 33
#define EN2 32
#define OE 37
#define RSTX 35
#define RSTY 30
#define X_Y 31

//Encoder counter results
uint32_t Result_lo;
uint32_t Result_3rd;
uint32_t Result_2nd;
uint32_t Result_hi;
int32_t Result;
double Angle1;
double Time0;
double Time1;
double Time2;
double Angle2;
double Speed;

// for incoming serial data
uint16_t incomingByte = 0;
uint16_t oldByte = 0;

//forward declare functions
uint8_t Get_Byte(uint8_t num);
double getAngle(int32_t count);
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
	Time0 = 0;
	Time1 = 0;
	Time2 = 0;
	Speed = 0;

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
    pinMode(D0,INPUT);
    pinMode(D1,INPUT);
    pinMode(D2,INPUT);
    pinMode(D3,INPUT);
    pinMode(D4,INPUT);
    pinMode(D5,INPUT);
    pinMode(D6,INPUT);
    pinMode(D7,INPUT);
		
    Serial.begin(9600);
    Serial.println("Enter 's' or 'S' to start, 'r' or 'R' to reset");
}


void loop()
{
    // if there is an incoming byte read it
    if (Serial.available() > 0) {
        oldByte = incomingByte;
        incomingByte = Serial.read();
		if(incomingByte != 114 && incomingByte != 82 && incomingByte!=115 && incomingByte!=83){
            Serial.println("Wrong Input: Enter 's' or 'S' to start, 'r' or 'R' to reset");
            incomingByte = oldByte;
        }
        else if(incomingByte==115 || incomingByte==83){
            Time0 = micros();
            Time1 = Time0;
            Time2 = Time0;
        }
    }
    
    // Reset of 'r' or 'R' is entered
    if (incomingByte == 114 || incomingByte == 82){
        Serial.print("Resetting... ");
        //Reset encoder counter
        digitalWrite(RSTX,LOW);
        digitalWrite(RSTY,LOW);
        delay(100);
        digitalWrite(RSTX,HIGH);
        digitalWrite(RSTY,HIGH);
        //Reset counts on arduino
        Result_lo = 0;
        Result_3rd = 0;
        Result_2nd = 0;
        Result_hi = 0;
        Result =0;
        Angle1 = 0;
        Angle2 = 0;
        Speed = 0;
        incomingByte = 0;
        Serial.println("Done.");
    }

	// Start Reading once 's' or 'S' is entered
	else if (incomingByte==115 || incomingByte==83){
		// Turn off read mode
		delay(50);

		// SEL1 and SEL2 for MSB
		digitalWrite(SEL1,LOW);
		digitalWrite(SEL2,HIGH);
		// Turn on read mode
		digitalWrite(OE,LOW);
		// Get MSB
		Result_hi = ((uint32_t) Get_Byte(3)) & 0x000000FF;

		// SEL1 and SEL2 for 2nd BYTE
		digitalWrite(SEL1,HIGH);
		digitalWrite(SEL2,HIGH);
		// Get 2nd Byte
		Result_2nd = ((uint32_t) Get_Byte(2)) & 0x000000FF;
		
		// sSEL1 and SEL2 for 3rd BYTE
		digitalWrite(SEL1,LOW);
		digitalWrite(SEL2,LOW);
		// Get 3rd Byte
		Result_3rd = ((uint32_t) Get_Byte(1)) & 0x000000FF;
		
		// SEL1 and SEL2 for LSB
		digitalWrite(SEL1,HIGH);
		digitalWrite(SEL2,LOW);
		// Get LSB
		Result_lo = ((uint32_t) Get_Byte(0)) & 0x000000FF;
		
		digitalWrite(OE,HIGH);
		
		//Combine Bytes to get 32 bit encoder count		
		Result = (int32_t) ((Result_hi << 24) + (Result_2nd << 16) + (Result_3rd << 8) + Result_lo);
		Angle2 = Angle1;
		Time2 = Time1;
		Angle1 = getAngle(Result);
		Time1 = (double) micros();
		Speed = getSpeed(Angle1,Angle2,Time1,Time2);
		Serial.print(Angle1);
		Serial.print(", ");
		Serial.print(Speed);
		Serial.print(", ");
		Serial.print(Time1 - Time0);
		Serial.print(", ");
		Serial.println(Time2 - Time0);
    }
}

uint8_t Get_Byte(uint8_t num)
{
	uint8_t Byte_old;
	uint8_t Byte_new;
	
	uint8_t temp0;
	uint8_t temp1;
	uint8_t temp2;
	uint8_t temp3;
	uint8_t temp4;
	uint8_t temp5;
	uint8_t temp6;
	uint8_t temp7;

	// Sample Data first time
	temp0 = digitalRead(D0);
	temp1 = digitalRead(D1);
	temp2 = digitalRead(D2);
	temp3 = digitalRead(D3);
	temp4 = digitalRead(D4);
	temp5 = digitalRead(D5);
	temp6 = digitalRead(D6);
	temp7 = digitalRead(D7);

	Byte_old = temp0;
	Byte_old |= temp1 << 1;
	Byte_old |= temp2 << 2;
	Byte_old |= temp3 << 3;
	Byte_old |= temp4 << 4;
	Byte_old |= temp5 << 5;
	Byte_old |= temp6 << 6;
	Byte_old |= temp7 << 7;
	
	// Sample Data second time
	temp0 = digitalRead(D0);
	temp1 = digitalRead(D1);
	temp2 = digitalRead(D2);
	temp3 = digitalRead(D3);
	temp4 = digitalRead(D4);
	temp5 = digitalRead(D5);
	temp6 = digitalRead(D6);
	temp7 = digitalRead(D7);

	Byte_new = temp0;
	Byte_new |= temp1 << 1;
	Byte_new |= temp2 << 2;
	Byte_new |= temp3 << 3;
	Byte_new |= temp4 << 4;
	Byte_new |= temp5 << 5;
	Byte_new |= temp6 << 6;
	Byte_new |= temp7 << 7;


	if(Byte_old == Byte_new)
	{	
		if (0)
		{
			Serial.print("Byte");
			Serial.println(num);
			Serial.print(temp7);
			Serial.print(temp6);
			Serial.print(temp5);
			Serial.print(temp4);
			Serial.print(temp3);
			Serial.print(temp2);
			Serial.print(temp1);
			Serial.println(temp0);
			Serial.println(Byte_new, HEX);
		}
		return Byte_new;
	}
	else
	{   
        delay(100);
		return Get_Byte(num);
	}
}


double getAngle(int32_t count)
{
	return ((double) count) * 2*PI/1440.;
	//return fmod((((double) count) * 360/1440.),360.);
}

double getSpeed(double angle1, double angle2, double time1, double time2)
{
	return (angle1 - angle2)*1.e6/(time1 - time2);
}
