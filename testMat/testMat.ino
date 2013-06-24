#include <stdint.h>
#include <math.h>

//serial output period in microsconds

typedef union{
    uint32_t num;
    uint8_t bytes[4];
} binaryFloat;

binaryFloat time;
uint32_t serialLastTime;
const uint16_t serialPer = 1e4;
float i = 0;
uint8_t calcChecksum(const uint8_t *vector, uint8_t len);

void setup()
{	
    //set up serial port
    time.num = 0;
	serialLastTime = 0;
    Serial.begin(115200);
	delay(1000);
}

void loop()
{	
    time.num = micros();
	if((time.num - serialLastTime) > serialPer)
    {
        //Serial.println(time.num);
        Serial.write(time.bytes,4);
        Serial.println();
        serialLastTime = time.num;
	}
}

//sum vector to obtain checksum
uint8_t calcChecksum(const uint8_t *vector, uint8_t len)
{
    uint8_t checksum = 0;
    for(uint8_t i = 0; i < len; i++){
        checksum += vector[i];
    }
    return checksum;
}
