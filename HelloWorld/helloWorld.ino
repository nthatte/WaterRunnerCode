/* Hello World program */

#include <stdint.h>
#include <stdio.h>

int i;
int led;

void setup()
{
    i = 0;
    led = 13;
    pinMode(led, OUTPUT);    
    digitalWrite(led,LOW);
	Serial.begin(9600);
    Serial.print("Please enter a number: ");
}

void loop()
{
    if(Serial.available())
    {
        i = Serial.parseInt();
        Serial.println("");
        Serial.print("You entered: ");
        Serial.println(i);
        Serial.print("Please enter a number: ");
        for(int num = 0; num < i; num++)
        {
            digitalWrite(led,HIGH);
            delay(250);
            digitalWrite(led,LOW);
            delay(250);
        }            
    }
}
