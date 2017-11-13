#ifndef _MESSAGE_WRITER_H_
#define _MESSAGE_WRITER_H_

#include <Arduino.h>

#define MAX_MESSAGE_LENGTH 128

struct message_output_t {
  char nodeId;
  char axisLetter;
  char commandCode;
  char * memoryBank;
  char * commandParam;
  char length;
};

void writerSendMessage(struct message_output_t *);
void writerPrepMessage(struct message_output_t *,char nodeId,
                       char axisLetter,char commandCode,
                       char * memoryBank,
                       char * commandParam);
#endif
