#include "MessageReader.h"
#include <Arduino.h>

void setupReader(struct message_t *message){
	message->data.length = 0;
}

uint8_t read_message(struct message_t *message) {

	switch (message->state) {
		case WAITING_FOR_MESSAGE: {
			if (Serial3.available() > 0) {
				message->data.unorganizedMessage[message->data.length] = (uint8_t)Serial3.read();
				Serial.println(message->data.unorganizedMessage[message->data.length]);
				message->data.length++;
				if (message->data.unorganizedMessage[message->data.length - 1] == '\r'){

						uint8_t currentBit = 0;
                        while (message->data.unorganizedMessage[currentBit] == ' '){
                            if (message->data.unorganizedMessage[currentBit] != '\r'){
                                message->data.commandCode[currentBit] = message->data.unorganizedMessage[2];
                                currentBit++;
                            }
                            message->data.commandCodeLength = currentBit;
                        }

						for (currentBit; currentBit<=message->data.length -1; currentBit++){
							message->data.commandParam[currentBit - message->data.commandCodeLength] = message->data.unorganizedMessage[currentBit + 1];

                        }
                        message->data.commandParamLength = currentBit - message->data.commandCodeLength;
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
