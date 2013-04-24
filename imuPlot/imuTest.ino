#include <stdint.h>
#include <math.h>

//IMU command bytes
const uint8_t packetStartNoHeader = 0xF7;
const uint8_t packetStartHeader = 0xF9;
const uint8_t responseHeaderConfig[] = {0xDD, 0x00, 0x00, 0x00, 0x09};
const uint8_t headerLength = 2;

//Timing variables for arduino --> computer communication
uint32_t Time1;
uint32_t serialLastTime = 0;
const uint32_t serialPer = 1e5;

//IMU results
const uint8_t responseLen = 18;
uint8_t rawQuatResponse[responseLen];
uint8_t *quaternion = &rawQuatResponse[headerLength];
char responseString[40];

//forwared declare functions
void setResponseHeader(const uint8_t *responseHeaderConfig);
void tareWithCurrentOrientation();
void getTaredQuaternion(uint8_t *rawQuatResponse);
void sendToComp(const uint8_t *data, uint8_t len);
bool goodResponse(const uint8_t *rawResponse, uint8_t len);
uint8_t calcChecksum(const uint8_t *vector, uint8_t len);

void setup()
{	
	//serial variables
    Serial.begin(115200);   //Serial connection for computer
    Serial1.begin(115200);  //Serial connection for imu
    delay(1000);
    setResponseHeader(responseHeaderConfig);
    Serial1.println(":116,2");
    tareWithCurrentOrientation();
    
    //Clear serial buffer on the IMU
    int burn = 0;
    while(burn != -1){
        burn = Serial1.read();
        delay(10);
    }
}


void loop()
{	
    getTaredQuaternion(rawQuatResponse);
    sendToComp(quaternion, 16); 
    delay(100);
}

void setResponseHeader(const uint8_t *responseHeaderConfig)
{
    //command for setting header
    Serial1.write(packetStartNoHeader);
    for(uint8_t i =0; i<5; i++){
        Serial1.write(responseHeaderConfig[i]);
    }
    Serial1.write(calcChecksum(responseHeaderConfig, 5));
    return;
}

void tareWithCurrentOrientation()
{
    //command for tare with current orientation
    int i = 0;
    const uint8_t command = 0x60;
    Serial1.write(packetStartNoHeader);
    Serial1.write(command);
    Serial1.write(command);  //checksum is same as command for a 1 byte command
    return;
}

void getTaredQuaternion(uint8_t *rawQuatResponse)
{   
    //command for getting tared quaternion
    const uint8_t command = 0x00;
    
    //Request tared orientation 
    do{
        Serial1.write(packetStartHeader);
        Serial1.write(command);
        Serial1.write(command); //checksum is same as command for a 1 byte command
        //Attempt to read until response is obtained
        Serial1.readBytes((char*) rawQuatResponse,responseLen); 
    } while(!goodResponse(rawQuatResponse, responseLen)); //Retry if response is not sucessful
 return;
}

void sendToComp(const uint8_t *quaternion, uint8_t len)
{
    Time1 = micros();
    
    //put time and data into one packet array
    uint8_t *timeArray = reinterpret_cast<uint8_t*>(&Time1);
    uint8_t packet[len + 4];

    //copy time
    for(uint8_t i = 0; i < 4; i++){
        packet[i] = timeArray[i];
    }

    //copy quaternion
    for(uint8_t i = 0; i < len ; i++){
        packet[i + 4] = quaternion[i];
    }
   
    Serial.write(packetStartNoHeader);
    Serial.write(packet, len+4);
    uint8_t checksum = calcChecksum(packet, len + 4);
    Serial.write(checksum);

    return;
}

// Interprets header to determine if reponse is successful
bool goodResponse(const uint8_t *rawResponse, uint8_t len)
{
    //check success/faliure byte. Nonzero numbers are faliures
    if(rawResponse[0] != 0x00){
        return false;
    }

    //validate checksum
    const uint8_t *data = &rawResponse[headerLength];
    if(rawResponse[1] != calcChecksum(data, len - headerLength)){
        return false;
    }

    return true;
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
