#ifndef PROCESS_MESSAGE_H_
#define PROCESS_MESSAGE_H_

#include <Arduino.h>

#define MAX_CHARACTER 6

struct set_t {
  uint8_t memoryBank;
  uint8_t parameterID;
  uint8_t value[MAX_CHARACTER-1];
};

struct get_t {
  uint8_t memoryBank;
  uint8_t parameterID[MAX_CHARACTER];
  uint8_t returnHex;
};

struct copy_t {
  uint8_t memoryBank;
  uint8_t parameterID;
};

struct tragectory_t {
  uint8_t commandCode;
};

struct registerReadWrite_t {
  uint8_t registerAccess;
  uint8_t value;
};

#endif
