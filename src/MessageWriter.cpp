#define WRITE_BUFFER_SIZE 256

#include "MessageWriter.h"

#define FinishBlock(X) (*code_ptr = (X), code_ptr = dst++, code = 0x01)

static void StuffData(const uint8_t *ptr, uint16_t length, uint8_t *dst)
{
  const uint8_t *end = ptr + length;
  uint8_t *code_ptr = dst++;
  uint8_t code = 0x01;

  while (ptr < end)
  {
    if (*ptr == 0){
      FinishBlock(code);
    }
    else {
      *dst++ = *ptr;
      code++;
    }
    ptr++;
  }
  FinishBlock(code);
  FinishBlock(0);

}

void writerSendMessage(struct message_output_t *message){
    /*uint8_t outputBuffer[message->length];
    outputBuffer[0] = message->nodeId;
    outputBuffer[1] = message->axisLetter;
    outputBuffer[2] = message->commandCode;
    for(int i=3; i < message->length; i++){
      outputBuffer[i] = message->commandParam[i - 3];
    }*/
    Serial.println("testibg");
    if(message->nodeId != NULL){
      Serial.print(message->nodeId);
      Serial.print(' ');
    }
    if(message->axisLetter != NULL){
      Serial.print(message->axisLetter);
      Serial.print(' ');
    }
    Serial.print(message->commandCode);
    Serial.print(' ');
    Serial.print(*message->memoryBank);
    Serial.print(' ');
    //Serial.println(*message->commandParam);
    //for (int i = 0; i<message->length; i++){
      //Serial.print(outputBuffer[i]);
      //Serial.print(" ");
    //}

    /*int i = 0;

    do {
      Serial.write(outputBuffer[i]);
      Serial.write(0x20);
      i++;
    } while(outputBuffer[i - 1] != 0);
    memset(message->commandParam, 0x00, MAX_MESSAGE_LENGTH - 3);
*/}

void writerPrepMessage(struct message_output_t *message,char nodeId,
                       char axisLetter,char commandCode,
                       char * memoryBank,
                       char * commandParam){

  message->nodeId = nodeId;
  message->axisLetter = axisLetter;
  message->commandCode = commandCode;
  message->memoryBank = *memoryBank;
  message->commandParam = commandParam;
  /*message->length = 3;
  for (int i=0; i < MAX_MESSAGE_LENGTH-3; i++){
    if (commandParam[i]){
      message->commandParam[i] = commandParam[i];
      message->length++;
    }
    else{
      break;
    }
  }*/
}
