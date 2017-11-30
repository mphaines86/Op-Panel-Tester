#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "MessageReader.h"
#include "storage.h"
#include "interface.h"
#include "inttypes.h"


#define DEBOUNCE_MAX 4

volatile uint32_t messageData = 0;
volatile uint32_t messageCount = 0;
uint8_t moveCommand = 0;
double forceConstant = 1;

struct message_t processMessage;

static double getForce(){

    double force = (7.464939541928912 * messageData) * 5.9 * 30 * 0.9 / forceConstant;
    return force;

}

void processBegin(){
    setupReader(&processMessage);
    message_output_t outputMessage {};
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "0");
    writerSendMessage(&outputMessage);
    delay(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x90", (char *) "115200");
    writerSendMessage(&outputMessage);
    Serial3.flush();
    Serial3.begin(115200);
    delay(1000);
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "21");
    //writerSendMessage(&outputMessage);
}

uint8_t processCalibrate() {

    tft.fillScreen(0x2924);

    message_output_t outputMessage {};

    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "21");
    writerSendMessage(&outputMessage);
    Delay_us(100);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xca", (char *) "0");
    writerSendMessage(&outputMessage);
    Delay_us(100);
    writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "r0x0c", (char *) nullptr);
    writerSendMessage(&outputMessage);
    Delay_ms(2000);

    while (messageCount < 3){

    }
    double force = getForce();

    forceConstant = 5 / force;

    return 1;

}

uint8_t processRun() {

    //message_output_t outputMessage{};

    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "21");
    //writerSendMessage(&outputMessage);
    //Delay_us(100);
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xc8", (char *) "0");
    //writerSendMessage(&outputMessage);
    //Delay_us(100);

    uint32_t currentIteration = 0;
    uint32_t runStart = (uint32_t) millis();

    processMove(parameterList[intMinAngle]);
    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Position setup");
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    tft.println("Please position op-panel\nin gripper and press\nany key to continue");

    while(true){
        int8_t keyValue = checkKeypad();
        if (keyValue >= 0){
            break;
        }
    }

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

    char currentFile[12];
    currentWorkingFile.toCharArray(currentFile, 12);
    storageWriteLine("SYSTEM_1.VAR", 1, currentFile);

    TIMSK3 |= (1 << OCIE3A);
    while (currentIteration < (uint32_t) parameterList[intCycle]) {
        Serial.println(parameterList[intCycle]);
        Serial.println(currentIteration);
        processMove(parameterList[intMaxAngle]);
        moveCommand = 1;
        uint8_t counter = 0;
        while (moveCommand | (PINA & (1 << PA3))) {
            delay(100);
            if (PINA & (1 << PA3)){
                moveCommand = 0;
            }

            /*if (!counter){
                if(currentIteration % parameterList[intStore] == 0){
                    writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "0x0c", (char * ) nullptr);
                    writerSendMessage(&outputMessage);
                    storageWriteToFile("RUNDATA1.TXT", messageData);
                }
                counter = 1;
            }*/

        }
        //Serial.println("delay");
        //delay(1000);
        processMove(parameterList[intMinAngle]);
        moveCommand = 1;
        counter = 0;
        while (moveCommand | (PINA & (1 << PA3))){
            delay(100);
            //Serial.print("2: ");
            //Serial.println(PINA & (1 << PA3));
            if(PINA & (1 << PA3)) {
                moveCommand = 0;
            }
            /*if (!counter){
                char *temp = nullptr;
                sprintf(temp, "%6d\n%8d", currentIteration, (uint32_t)
                        ((millis() - runStart) / 1000));
                storageWriteLine("SYSTEM_1.VAR", 2, temp);
                counter = 1;
            }*/
        }
        //Serial.println("delay");
        //delay(1000);
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_BLACK);
        tft.print(currentIteration);
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_WHITE);
        currentIteration++;
        tft.print(currentIteration);
    }

    TIMSK3 &= (0 << OCIE3A);

    return 1;
}

uint8_t processHelp() {

    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Position setup");
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    tft.print("Last File Ran:");
    tft.print(storageReadLine("SYSTEM_1.VAR", 1));
    tft.print("Last Cycle:");
    tft.print(storageReadLine("SYSTEM_1.VAR", 2));
    tft.print("Last Run Time:");
    tft.print(storageReadLine("SYSTEM_1.VAR", 3));

    return 1;
}

uint8_t processAttributes() {
    return 1;
}

uint8_t processSave() {
    storageSaveParameters();
    return 1;
}

uint8_t processNew(){
    storageNewFile();
    return 1;
}

uint8_t processLoad() {

    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println("Select File");
    tft.setTextSize(2);
    uint16_t currentFileList = 0;
    Array fileList;
    initArray(&fileList, 1);
    uint8_t totalFiles = storageGetFiles(&fileList);
    if(!totalFiles)
        return 0;
    for (int i = currentFileList; i < 6; ++i) {
        if (i == totalFiles)
            break;
        for (uint8_t j = 0; j < 12; ++j){
            tft.print(fileList.array[j + i * 12]);
        }
        tft.println();
    }
    int8_t keyboardValue;
    while(true){
        keyboardValue = checkKeypad();
        keyboardValue = handleUserInput(keyboardValue);
        if (keyboardValue >= 1 && keyboardValue <= 6){
            char desiredFile[12];
            for (uint8_t i =0; i < 12; i++) {
                desiredFile[i] = fileList.array[(currentFileList + keyboardValue -1) * 12 + i];
            }
            Serial.println(desiredFile);
            storageLoadSD((String) desiredFile);
            freeArray(&fileList);
            return 1;
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
            freeArray(&fileList);
            return 0;
        }
    }
}

uint8_t processMove(uint8_t degree){
    uint32_t steps = (uint32_t) degree * 961;

    /*if (steps < 0){
        PORTE &= ~(1 << PE3);
        steps = steps * -1;
    }
    else{
        PORTE |= (1 << PE3);
    }


    PORTA |= (1 << PA0);*/
    PORTA |= (1 << PA1);

    char temp[6];

    sprintf(temp, "%"PRIu32"", steps);
    message_output_t outputMessage{};
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xca", (char *) temp);
    writerSendMessage(&outputMessage);
    delayMicroseconds(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 't', (char *) "1", (char *) nullptr);
    writerSendMessage(&outputMessage);
    delayMicroseconds(1000);

    /* uint16_t count = 0;
    steps++;
    while (count < (uint16_t) steps){
        Serial.print(count);
        Serial.print(" ");
        PORTH |= (1 << PH3);
        delayMicroseconds(2000);
        PORTH &= ~(1 << PH3);
        delayMicroseconds(2000);
        count++;
    }
    Serial.println();*/
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

    return 1;
}


ISR(TIMER3_COMPA_vect) {
    if(read_message(&processMessage)){
        if (processMessage.data.commandCode[0] == 'v'){
            int i = 0;
            while (i < processMessage.data.commandParamLength){
                messageData = messageData * 10 + (processMessage.data.commandParam[i] - '0');
            }
        }
        messageCount++;
        message_processed(&processMessage);
    }
}

