#include "MessageWriter.h"

void writerSendMessage(struct message_output_t *message){
    /*uint8_t outputBuffer[message->length];
    outputBuffer[0] = message->nodeId;
    outputBuffer[1] = message->axisLetter;
    outputBuffer[2] = message->commandCode;
    for(int i=3; i < message->length; i++){
      outputBuffer[i] = message->commandParam[i - 3];
    }*/
    //Serial3.write(message->commandParam, HEX);
    //Serial.write(message->commandParam, HEX);
    //if(message->nodeId != '\0'){
    //  Serial3.write(message->nodeId);
    //  Serial3.write(' ');
    //}
    //if(message->axisLetter != '\0'){
    //  Serial3.write(message->axisLetter);
    //  Serial3.write(' ');
    //}
    //Serial3.print(message->commandCode);
    //Serial3.print(' ');
    //Serial3.print(message->memoryBank);
    //Serial3.print(' ');

    Serial3.print('r');
    //Serial3.print(' ', DEC);
    //Serial3.print('r', DEC);
    //Serial3.print('0', DEC);
    //Serial3.print('x', DEC);
    //Serial3.print('1', DEC);
    //Serial3.print('8', DEC);

    if(message->commandParam != nullptr) {
        Serial3.print(message->commandParam);
    }
    Serial3.print('\r');

    Serial.print(message->commandCode);
    Serial.print(' ');
    Serial.print(message->memoryBank);
    Serial.print(' ');

    if(message->commandParam != nullptr) {
        Serial.print(message->commandParam);
    }
    Serial.print('\r');
    Serial.println();
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
  message->memoryBank = memoryBank;
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
