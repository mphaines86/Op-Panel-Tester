#include "MessageReader.h"
#include <Arduino.h>

// #define DEBUG_READER 1

void setupReader(struct message_t *message){
	message->data.length = 0;
}

uint8_t read_message(struct message_t *message) {

	switch (message->state) {
		case WAITING_FOR_MESSAGE: {
			if (Serial3.available() > 0) {
				message->data.unorganizedMessage[message->data.length] = (uint8_t)Serial3.read();
#ifdef DEBUG_READER
				Serial.println(message->data.unorganizedMessage[message->data.length]);
#endif
				message->data.length++;
				if (message->data.unorganizedMessage[message->data.length - 1] == '\r'){

						uint8_t currentBit = 0;
                        while (message->data.unorganizedMessage[currentBit] != ' ' &&  message->data.unorganizedMessage[currentBit] != '\r') {
                            message->data.commandCode[currentBit] = message->data.unorganizedMessage[currentBit];
                            currentBit++;
                            message->data.commandCodeLength = currentBit;
                        }

                        if (message->data.unorganizedMessage[currentBit] == '\r'){
                            message->state = MESSAGE_READY;
                            message->data.commandParamLength = 0;
                            message->data.length = 0;
                            break;
                        }
                        else if (message->data.unorganizedMessage[currentBit] == ' '){
                            currentBit ++;
                        }

						for (; currentBit<=(message->data.length -2); currentBit++){
							message->data.commandParam[currentBit - message->data.commandCodeLength - 1] = message->data.unorganizedMessage[currentBit];

                        }
                        message->data.commandParamLength = currentBit - message->data.commandCodeLength - 1;
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
