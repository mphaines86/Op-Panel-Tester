#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"
#include "utilities.h"

extern uint8_t storage_SD_loaded;

typedef enum {
  intSpeed=0, intMaxAngle, intMinAngle, intCycle, intMaxForce, intMinForce,
  intError, intStore, intCount,
} intParameter_t;

extern uint16_t parameterList[intCount];

typedef enum {
actCal=0, actTest, actAtt, actHelp, actSave, actLoad, actNew, actHome, actCount
} actionParameter_t;

typedef enum {
boolFail=0, boolMove, boolCount
} booleanParameter_t;

extern uint8_t booleanList[boolCount];

uint8_t storageSaveParameters();
uint8_t storageBeginSD();
uint8_t storageLoadSD(const String &fileName);
void storageNewFile();
uint8_t storageGetFiles(Array * listOfFiles);
uint8_t storageWriteToFile(const String &fileName,
                           uint32_t data);
#endif
