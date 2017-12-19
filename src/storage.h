#ifndef STORAGE_H
#define STORAGE_H

// #define DEBUG 1

#include "Arduino.h"
#include "utilities.h"

extern uint8_t storage_SD_loaded;
extern String currentWorkingFile;

typedef enum {
  intSpeed=0, intMaxAngle, intMinAngle, intCycle, intMaxForce, intMinForce,
  intError, intStore, intDelay, intCount,
} intParameter_t;

extern uint32_t parameterList[intCount];

typedef enum {
actCal=0, actTest, actAtt, actHelp, actSave, actLoad, actNew, actHome, actCount
} actionParameter_t;

typedef enum {
boolFail=0, boolMove, boolCount
} booleanParameter_t;

extern uint8_t booleanList[boolCount];

uint8_t storageSaveParameters();
uint8_t storageBeginSD();
String storageReadLine(const String &fileName, uint16_t lineNumber);
uint8_t storageLoadSD(const String &fileName);
uint8_t storageWriteLine(const String &fileName, uint16_t lineNumber, char * data);
void storageNewFile();
uint8_t storageGetFiles(Array * listOfFiles);
uint8_t storageWriteToFile(const String &fileName,
                           uint32_t data);
#endif
