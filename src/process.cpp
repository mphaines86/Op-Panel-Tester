#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "storage.h"
#include "interface.h"


#define DEBOUNCE_MAX 4

double armPosition = 0;

void processBegin(){

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

    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Testing in Progress");
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    tft.println("Current Cycle Number:");
    tft.print(currentIteration);

    while (currentIteration < parameterList[intCycle]) {
        processMove(parameterList[intMaxAngle]);
        while(PINA & (1 << PA3)){

        }
        delay(100);
        processMove(parameterList[intMinAngle]);
        while(PINA & (1 << PA3)){

        }
        delay(100);
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_BLACK);
        tft.print(currentIteration);
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_WHITE);
        currentIteration++;
        tft.print(currentIteration);
    }

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
        tft.fillRect(60, i * 6, tft.width(), 8, colorBar[i - 8]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Select File");
    tft.setTextSize(2);
    uint16_t currentFileList = 0;
    Array fileList;
    initArray(&fileList, 1);
    storageGetFiles(&fileList);

    for (int i = currentFileList; i < 6; ++i) {
        tft.println(fileList.array[i]);
    }
    int8_t keyboardValue;
    while(true){
        keyboardValue = checkKeypad();
        if (keyboardValue >= 1 && keyboardValue <= 6){
            storageLoadSD(String(fileList.array[currentFileList + keyboardValue]));
            return 0;
        }
        if (keyboardValue == 7){
            if (currentFileList == 0){
            }
            else{
                currentFileList-=5;
                tft.setCursor(0,90);
                for (int i = currentFileList; i < 6; ++i) {
                    tft.println(fileList.array[i]);
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
                    tft.println(fileList.array[i]);
                }
            }
        }
        if (keyboardValue == 11){

            return 0;
        }
    }
}

uint8_t processMove(uint16_t degree){
    auto steps = static_cast<int16_t>((degree - armPosition) / 0.406779661017);

    armPosition = degree;

    if (steps < 0){
        PORTE &= ~(1 << PE3);
        steps = steps * -1;
    }
    else{
        PORTE |= (1 << PE3);
    }

    Serial.println(steps);

    PORTA |= (1 << PA0);
    PORTA |= (1 << PA1);

    uint16_t count = 0;
    steps++
    while (count < (uint16_t) steps){
        Serial.print(count);
        Serial.print(" ");
        PORTH |= (1 << PH3);
        delayMicroseconds(1000);
        PORTH &= ~(1 << PH3);
        delayMicroseconds(1000);
        count++;
    }
    Serial.println();
    PORTA &= ~(1 << PA1);

    return 1;
}

uint8_t processHome(){
    PORTA &= ~(1 << PA0);
    PORTA |= (1 << PA1);
    while(!(PINA & (1 << PA2))){

    }
    PORTA |= (1 << PA0);
    PORTA &= ~(1 << PA1);
    armPosition = 0;

    return 1;
}


ISR(TIMER3_COMPA_vect) {
    Serial.println("NO");
}

