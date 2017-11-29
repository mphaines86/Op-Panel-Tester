#ifndef COMMUNICATE_MESSAGE_H_
#define COMMUNICATE_MESSAGE_H_

#include <Arduino.h>

#define MAX_MESSAGE_LENGTH 10

enum message_state_t {
	WAITING_FOR_MESSAGE,
	MESSAGE_READY,
	MESSAGE_FAILED,
};

struct message_data_t {
    uint8_t commandCode[MAX_MESSAGE_LENGTH];
	uint8_t commandCodeLength;
	uint8_t commandParam[MAX_MESSAGE_LENGTH];
	uint8_t commandParamLength;
	uint8_t unorganizedMessage[MAX_MESSAGE_LENGTH];
	uint8_t length;
};

struct message_t {
	enum message_state_t state;
	struct message_data_t data;
};

void setupReader(struct message_t *);
uint8_t read_message(struct message_t *);
void message_processed(struct message_t *);

#endif // COMMUNICATE_MESSAGE_H_
