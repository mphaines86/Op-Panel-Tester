#include "MessageReader.h"
#include <Arduino.h>

void setupReader(struct message_t *message){
	message->data.length = 0;
}

uint8_t read_message(struct message_t *message) {

	//Serial.println(message->state);
	switch (message->state) {
		case WAITING_FOR_MESSAGE: {
			if (Serial.available() > 0) {
				message->data.unorganizedMessage[message->data.length] = Serial.read();
				message->data.length++;
				if (message->data.unorganizedMessage[message->data.length - 1] == 0xD){

						message->data.header.nodeID=message->data.unorganizedMessage[0];
						message->data.header.axisLetter=message->data.unorganizedMessage[1];
						message->data.header.commandCode=message->data.unorganizedMessage[2];

						for (int i=3; i<=message->data.length; i++){
							message->data.commandParam[i - 3] = message->data.unorganizedMessage[i];
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