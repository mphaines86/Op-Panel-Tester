#ifndef _MESSAGE_WRITER_H_
#define _MESSAGE_WRITER_H_

#include <Arduino.h>

#define MAX_MESSAGE_LENGTH 10

struct message_output_t {
  uint8_t nodeId;
  uint8_t axisLetter;
  uint8_t commandCode;
  uint8_t commandParam[MAX_MESSAGE_LENGTH-3];
  uint8_t length;
};

void writerSendMessage(struct message_output_t *);
void writerPrepMessage(struct message_output_t *, uint8_t nodeId,
                       uint8_t axisLetter, uint8_t commandCode,
                       uint8_t commandParam[MAX_MESSAGE_LENGTH - 3]);
#endif
