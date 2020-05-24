#include <arduino.h>

void setup(void)
{
    Serial.begin(115200);
    while(!Serial)
        ;
    Serial.println("Setup");
}

void loop(void)
{
}
