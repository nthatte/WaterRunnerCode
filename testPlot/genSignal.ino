float t;
double y;

void setup()
{
	Serial.begin(9600);
    delay(1000);
}

void loop()
{
    t = (float) millis()/1000.;
    y = sin(5*t);
    Serial.print(t);
    Serial.print(" ");
    Serial.println(y);
    delay(1);
}
