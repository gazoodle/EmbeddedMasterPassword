#include <Arduino.h>
#include <Streaming.h>
#include "src/lib/mpw.h"

MPW mpw;

void setup(void)
{
    Serial.begin(115200);
    while(!Serial)
        ;
    Serial.println("Setup");
    uint8_t last_percent = 0;
    mpw.login("user","password",[&](uint8_t percent){
        if ( percent != last_percent )
        {
            last_percent = percent;
            Serial.print(".");
        }
    });
    Serial.println("");
    Serial.println("Complete");
}

void loop(void)
{
}
