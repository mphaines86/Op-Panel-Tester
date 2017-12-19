#include "MessageWriter.h"
// #define DEBUG_WRITER 1

void writerSendMessage(struct message_output_t *message){

    if(message->nodeId != '\0'){
      Serial3.write(message->nodeId);
      Serial3.write(' ');
    }
    if(message->axisLetter != '\0'){
      Serial3.write(message->axisLetter);
      Serial3.write(' ');
    }
    Serial3.print(message->commandCode);
    Serial3.print(' ');
    Serial3.print(message->memoryBank);
    Serial3.print(' ');

    if(message->commandParam != nullptr) {
        Serial3.print(message->commandParam);
    }
    Serial3.print('\r');

    delay(1);

#ifdef DEBUG_WRITER
    Serial.print(message->commandCode);
    Serial.print(' ');
    Serial.print(message->memoryBank);
    Serial.print(' ');

    if(message->commandParam != nullptr) {
        Serial.print(message->commandParam);
    }
    Serial.print('\r');
    Serial.println();
#endif
}

void writerPrepMessage(struct message_output_t *message,char nodeId,
                       char axisLetter,char commandCode,
                       char * memoryBank,
                       char * commandParam){

  message->nodeId = nodeId;
  message->axisLetter = axisLetter;
  message->commandCode = commandCode;
  message->memoryBank = memoryBank;
  message->commandParam = commandParam;
}
