#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "storage.h"
#include "interface.h"

volatile uint8_t encoderPinALast = 0;
volatile int32_t encoderPos = 0;

void processBegin(){
    /*message_output_t outputMessage{};
    // writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "s r0x90 00 07 03 0d 00 90 00 01 c2 00 \r");
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x90", (char *) "115200");
    writerSendMessage(&outputMessage);
    Serial3.flush();
    Serial3.begin(115200);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "1000");
    writerSendMessage(&outputMessage);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "1");
    writerSendMessage(&outputMessage);
    delay(1000);
    /*while(true){
        writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "r0x02 0");
        writerSendMessage(&outputMessage);
        delay(1000);
        writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "r0x02 -1000");
        writerSendMessage(&outputMessage);
        delay(1000);
        writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "r0x02 0");
        writerSendMessage(&outputMessage);
        delay(1000);
        writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "r0x02 1000");
        writerSendMessage(&outputMessage);
        delay(1000);
    }*/

}

uint8_t processCalibrate() {
    uint8_t cycle = 0;
    TIMSK3 |= (1 << OCIE3A);
    while (cycle < 5) {
        Delay_ms(1000);
        cycle++;
    }
    Serial.println("Testing Functions!!!");
    TIMSK3 &= (0 << OCIE3A);
    return 1;
}

uint8_t processRun() {

    uint32_t currentIteration = 0;

    for (int i = 8; i <= 13; i++)
        tft.fillRect(60, i * 6, 340, 8, colorBar[i - 8]);

    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Testing in Progress");
    tft.setTextSize(1);
    tft.println();
    tft.println("Current Cycle Number:");
    tft.setCursor(0,67);
    tft.print(currentIteration);

    analogWrite(6, 0);
    message_output_t outputMessage{};
    writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "r0x18", (char *) nullptr);
    writerSendMessage(&outputMessage);
    delay(5000);
    analogWrite(6, 127);
    delay(1000);
    analogWrite(6, 255);
    delay(2000);

    //processBegin();
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "100");
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "1");
    //writerSendMessage(&outputMessage);
    return 1;
}

uint8_t processHelp() {
    return 1;
}

uint8_t processAttributes() {
    return 1;
}

uint8_t processSave() {
    storageSaveParameters();
    return 1;
}

uint8_t processLoad() {

    tft.fillScreen(0x2924);
    for (int i = 8; i <= 13; i++)
        tft.fillRect(60, i * 6, 340, 8, colorBar[i - 8]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Select File");
    tft.setTextSize(1);
    uint8_t currentFileList = 0;
    /*String *fileList = storageGetFiles();

    for (int i = currentFileList; i < 6; ++i) {
        tft.println(fileList[i]);
    }
    int8_t keyboardValue;
    while(true){
        keyboardValue = checkKeypad();
        if (keyboardValue >= 1 && keyboardValue <= 6){
            storageLoadSD(fileList[currentFileList + keyboardValue]);
            return 0;
        }
        if (keyboardValue == 7){
            if (currentFileList == 0){
            }
            else{
                currentFileList-=5;
                tft.setCursor(0,90);
                for (int i = currentFileList; i < 6; ++i) {
                    tft.println(fileList[i]);
                }
            }
        }
        if (keyboardValue == 9){
            if(currentFileList ==250){
            }
            else{
                currentFileList+=5;
                tft.setCursor(0,90);
                for (int i = currentFileList; i < 6; ++i) {
                    tft.println(fileList[i]);
                }
            }
        }
        if (keyboardValue == 11){
            return 0;
        }
    }*/
    return 0;
}

const uint8_t fsrPin = 0;
volatile int16_t fsrReading;     // the analog reading from the FSR resistor divider
volatile int16_t fsrVoltage;     // the analog reading converted to voltage
volatile uint32_t fsrResistance;  // The voltage converted to resistance, can be very big so make "long"
volatile uint32_t fsrConductance;
volatile int32_t fsrForce;       // Finally, the resistance converted to force

ISR(TIMER3_COMPA_vect) {
    Serial.println("NO");
}

ISR(INT4_vect){
    uint8_t a;
    a = static_cast<uint8_t>((PINE & (1 << PINE4)) >> 4);
    uint8_t b;
    b = static_cast<uint8_t >((PINE & (1 << PINE5)) >> 5);
    if ((b == HIGH) && (a == HIGH)) {
        //if (b == LOW) {
        encoderPos--;
        Serial.println('B');
    } else {
        Serial.println('F');
        encoderPos++;
    }
    //Serial.println(encoderPos);
        //Serial.print ("/");
    // }
    // encoderPinALast = a;

}
