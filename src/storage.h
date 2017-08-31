#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"

extern uint8_t storage_SD_loaded;

typedef enum {
  intSpeed=0, intMaxAngle, intMinAngle, intCycle, intMaxForce, intMinForce,
  intError, intStore, intCount,
} intParameter_t;

extern uint16_t parameterList[intCount];

typedef enum {
actCal=0, actTest, actAtt, actHelp, actSave, actLoad, actCount
} actionParameter_t;

typedef enum {
boolFail=0, boolCount
} booleanParameter_t;

extern uint8_t booleanList[boolCount];

uint8_t storageSaveParameters();
uint8_t storageBeginSD();
uint8_t storageLoadSD(String fileName);
void storageNewFile();
String* storageGetFiles();

#endif
