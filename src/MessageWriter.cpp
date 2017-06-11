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
    uint8_t outputBuffer[message->length], writeBuffer[WRITE_BUFFER_SIZE];
    memset(writeBuffer, 0x3E, WRITE_BUFFER_SIZE);
    outputBuffer[0] = message->length;
    outputBuffer[1] = message->action;
    for(int i=2; i < message->length; i++){
      outputBuffer[i] = message->body[i - 2];
    }

    //for (int i = 0; i<message->length; i++){
      //Serial.print(outputBuffer[i]);
      //Serial.print(" ");
    //}
    //Serial.println("");
    //StuffData(outputBuffer, message->length, writeBuffer);

    int i = 0;

    do {
      Serial.write(writeBuffer[i]);
      //Serial.print(" ");
      i++;
    } while(writeBuffer[i - 1] != 0);

    //Serial.println("");
}

void writerPrepMessage(struct message_output_t *message, uint8_t command, uint8_t body[MAX_MESSAGE_SIZE - 2]){

  message->action = command;
  switch (command){
    case 't':{
      message->length = 6;
      for(int i = 0; i < 6; i++){
        message->body[i] = body[i];
      }
      break;
    }
    case 'p':{
      message->length = 3;
      message->body[0] = body[0];
      break;
    }
  }
}
