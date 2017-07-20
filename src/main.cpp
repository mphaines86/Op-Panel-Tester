#include <Arduino.h>
#include "interface.h"
#include "process.h"

void setup();
void loop();

int main(){
	init();

	#if defined(USBCON)
		USB.attach();
	#endif

		setup();

		while(1) {
			loop();
			if (serialEventRun) serialEventRun();
		}

		return 0;
}

void setup(void) {
  Serial.begin(9600);

	for(int i=47; i>=41; i-=2){ //rows
		pinMode(i, OUTPUT);
	}
	for(int i=39; i>=33; i-=2){ //columns
		pinMode(i, INPUT_PULLUP);
	}

	cli();

	TCCR3A = 0;// set entire TCCR1A register to 0
	TCCR3B = 0;// same for TCCR1B
	TCNT3  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments
	OCR3A = F_CPU / 200;// = (16*10^6) / (1*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR3B |= (1 << WGM32);
	// Set CS10 and CS12 bits for 1024 prescaler
	TCCR3B |= (1 << CS32) | (0 << CS31) | (1 << CS30);
	// enable timer compare interrupt

	sei();
	interfaceInit();
}

void loop(void){
	interfaceCheck();
}
