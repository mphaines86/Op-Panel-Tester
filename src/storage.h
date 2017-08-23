#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"

typedef enum {
  intSpeed=0, intMaxAngle, intMinAngle, intCycle, intMaxForce, intMinForce,
  intError, intStore, intCount,
} intParameter_t;

extern uint16_t parameterList[intCount];

typedef enum {
actCal=0, actTest, actAtt, actHelp, actCount, actSave, actLoad
} actionParameter_t;

typedef enum {
boolFail=0, boolCount
} booleanParameter_t;

extern uint8_t booleanList[boolCount];

#endif
