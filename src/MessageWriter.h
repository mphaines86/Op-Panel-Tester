#ifndef _MESSAGE_WRITER_H_
#define _MESSAGE_WRITER_H_

#include <Arduino.h>

#define MAX_MESSAGE_SIZE 254

struct message_output_t {
  uint8_t length;
  uint8_t action;
  uint8_t body[MAX_MESSAGE_SIZE - 2];
};

void writerSendMessage(struct message_output_t *);
void writerPrepMessage(struct message_output_t *, uint8_t command, uint8_t body[MAX_MESSAGE_SIZE - 2]);



#endif
