#ifndef COMMUNICATE_MESSAGE_H_
#define COMMUNICATE_MESSAGE_H_

#include <Arduino.h>

#define MAX_MESSAGE_LENGTH 256
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

enum message_state_t {
	WAITING_FOR_MESSAGE,
	MESSAGE_READY,
	MESSAGE_FAILED,
};

struct message_data_t {
	enum message_state_t state;
	struct {
		uint8_t length;
		uint8_t action;
	} header;
	uint8_t length;
	uint8_t body[MAX_MESSAGE_LENGTH];
	uint8_t stuffed_body[MAX_MESSAGE_LENGTH];
	uint8_t unstuffed_body[MAX_MESSAGE_LENGTH - 2];
};

struct message_t {
	enum message_state_t state;
	struct message_data_t data;
};

void setupReader(struct message_t *);
uint8_t read_message(struct message_t *);
void message_processed(struct message_t *);

#endif // COMMUNICATE_MESSAGE_H_
