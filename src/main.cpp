#include <Arduino.h>
#include "interface.h"
#include "SD.h"
#include "MessageReader.h"
#include "MessageWriter.h"

int main() {
    init();

#if defined(USBCON)
    USB.attach();
#endif

    setup();

    while (1) {
        loop();
        if (serialEventRun) serialEventRun();
    }

    return 0;
}

struct message_t message;

void setup(void) {
    Serial3.begin(9600);
    Serial.begin(115200);

    pinMode(6, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);
    pinMode(10, INPUT);
    pinMode(11, INPUT);

    for (uint8_t i = 47; i >= 41; i -= 2) { //rows
        pinMode(i, OUTPUT);
    }
    for (uint8_t i = 39; i >= 33; i -= 2) { //columns
        pinMode(i, INPUT_PULLUP);
    }

    cli();

    TCCR3A = 0;// set entire TCCR1A register to 0
    TCCR3B = 0;// same for TCCR1B
    TCNT3 = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR3A = (uint16_t) (F_CPU /
                        200);// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR3B |= (1 << WGM32);
    // Set CS10 and CS12 bits for 1024 prescaler
    TCCR3B |= (1 << CS32) | (0 << CS31) | (1 << CS30);
    // enable timer compare interrupt

    EICRB |= (1 << ISC40) | (1 << ISC41);
    EIMSK |= (1 << INT4);

    sei();


    analogWrite(6, 255);

    setupReader(&message);
    interfaceInit();
}

void loop(void) {
    interfaceCheck();
    if (read_message(&message)) {
        //process_message(&message);
    }

}
