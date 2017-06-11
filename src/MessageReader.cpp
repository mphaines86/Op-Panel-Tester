#include "MessageReader.h"
#include <Arduino.h>

static void COBSReader(const uint8_t *stuffed, uint8_t length, uint8_t *unstuffed){
	const uint8_t *end = stuffed + length - 1;
	while (stuffed < end){
		int i, code = *stuffed++;
		for(i=1; stuffed<end && i<code; i++){
			*unstuffed++ = *stuffed++;
		}
		if (code < 0xFF){
			*unstuffed++=0;
		}
	}
}

void setupReader(struct message_t *message){
	message->data.length = 0;
}

uint8_t read_message(struct message_t *message) {

	//Serial.println(message->state);
	switch (message->state) {
		case WAITING_FOR_MESSAGE: {
			if (Serial.available() > 0) {
				message->data.stuffed_body[message->data.length] = Serial.read();
				message->data.length++;
				if (message->data.stuffed_body[message->data.length - 1] == 0){

						//COBSReader(message->data.stuffed_body, message->data.length, message->data.unstuffed_body);

						message->data.header.length = message->data.unstuffed_body[0];
						//Serial.print(message->data.unstuffed_body[0]);
						//Serial.print(" ");
						message->data.header.action = message->data.unstuffed_body[1];
						//Serial.print(message->data.unstuffed_body[1]);
						//Serial.print(" ");
						for (int i=2; i<=message->data.header.length - 1; i++){
							message->data.body[i - 2] = message->data.unstuffed_body[i];
							//Serial.print(message->data.body[i - 2]);
							//Serial.print(" ");
						}
						//Serial.println("");
						message->state = MESSAGE_READY;
						message->data.length = 0;
				}
			}
			break;
		}

		case MESSAGE_READY: {
			message->data.length = 0;
			break;
		}

		case MESSAGE_FAILED: {
			Serial.println("ERROR!!! Failed to recieve message.");
			message->state = WAITING_FOR_MESSAGE;
			break;
		}
	}
	return message->state == MESSAGE_READY;
}

void message_processed(struct message_t *message) {
	message->state = WAITING_FOR_MESSAGE;
}
