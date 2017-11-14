#ifndef INTERFACE_H
#define INTERFACE_H

#include <Arduino.h>
#include "../lib/Adafruit_HX8357/Adafruit_HX8357.h"

#define NUM_OF_ROWS 4
#define NUM_OF_COLUMNS 4

extern const uint16_t colorBar[];

extern Adafruit_HX8357 tft;
void interfaceCheck();
void interfaceInit();
int8_t checkKeypad();

#endif
