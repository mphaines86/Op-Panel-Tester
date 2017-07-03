#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"

typedef enum {
  intSpeed=0, intMaxAngle, intMinAngle, intCycle, intMaxForce, intMinForce,
  intError, intStore, intCount,
} intParameter_t;

uint16_t parameterList[intCount];

typedef enum {
actCal=0, actTest, actAtt, actHelp, actCount
} actionParameter_t;

typedef void (*functionPtr_t)(void);

functionPtr_t functionList[actCount];

typedef enum {
boolFail=0, boolStore, boolCount
} booleanParameter_t;

uint8_t booleanList[boolCount];

#endif
