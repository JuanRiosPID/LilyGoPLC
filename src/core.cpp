#include "core.h"
#include "screen.h"
#include "serialcoms.h"

void core_setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("System started!");
    screen_setup();
    serialcoms_setup();
}

void core_loop()
{
    screen_loop();
    serialcoms_loop();
}