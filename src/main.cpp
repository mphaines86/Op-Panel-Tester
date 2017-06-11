#include <Arduino.h>
#include "interface.h"

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

	interfaceInit();

}

void loop(void){
	interfaceCheck();
}
