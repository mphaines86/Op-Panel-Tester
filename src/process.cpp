#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "MessageReader.h"
#include "storage.h"
#include "interface.h"
#include "inttypes.h"


#define DEBOUNCE_MAX 4
#define NUMBER_OF_SAMPLES 6

uint32_t ampData[NUMBER_OF_SAMPLES];
volatile uint32_t messageData = 0;
volatile uint32_t messageCount = 0;
volatile uint8_t interuptCounter = 0;
volatile uint8_t interuptCounter2 = 0;
uint8_t delayActive = 0;
uint8_t moveCommand = 0;
double forceConstant = 1;

struct message_t processMessage;

static double getForce(){

    double force = (7.464939541928912 * messageData) * 5.9 * 30 * 0.9 / forceConstant;
    return force;

}

void processBegin(){
    for (int i = 0; i < NUMBER_OF_SAMPLES; ++i) {
        ampData[i] = 0;
    }

    setupReader(&processMessage);
    message_output_t outputMessage {};
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x02", (char *) "0");
    //writerSendMessage(&outputMessage);
    delay(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x90", (char *) "115200");
    writerSendMessage(&outputMessage);
    Serial3.flush();
    Serial3.begin(115200);
    delay(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "21");
    writerSendMessage(&outputMessage);
    delay(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xc8", (char *) "0");
    writerSendMessage(&outputMessage);
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

    TIMSK3 |= (1 << OCIE3A);

    message_output_t outputMessage{};

    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0x24", (char *) "21");
    writerSendMessage(&outputMessage);
    delayMicroseconds(5000);
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xc8", (char *) "0");
    //writerSendMessage(&outputMessage);
    delayMicroseconds(5000);
    char temp[8];
    sprintf(temp, "%"PRIu32"", parameterList[intSpeed] * 10000);
    Serial.println(temp);


    processMove(parameterList[intMinAngle]);
    delay(1000);
    writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) "r0xcb", (char *) temp);
    writerSendMessage(&outputMessage);

    uint32_t currentIteration = 0;
    uint32_t lastDelay = 0;

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
    tft.println(currentIteration);
    tft.println("Average Amperage (mA):");
    tft.print(0);

    auto runStart = (uint32_t) millis();

    while (currentIteration < parameterList[intCycle]) {
        Serial.print("iter:");
        Serial.println(currentIteration);
        processMove(parameterList[intMaxAngle]);
        moveCommand = 1;
        uint8_t counter = 0;
        while (moveCommand | (PINA & (1 << PA3))) {
            //Serial.print(1);
            delay(100);
            if (PINA & (1 << PA3)){
                moveCommand = 0;
            }

            if (!counter){
                char charHold[20];
                sprintf(charHold, "%09"PRIu32"\n%09"PRIu32"\n", currentIteration, (uint32_t) (((uint32_t)millis() - runStart)/1000));
                storageWriteLine("SYSTEM_1.VAR", 0, charHold);
            }
            if (counter < (NUMBER_OF_SAMPLES/2)){
                writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "r0x0c", (char * ) nullptr);
                writerSendMessage(&outputMessage);
                counter++;
            }
        }

        delayActive = 1;

        //uint32_t tmp[NUMBER_OF_SAMPLES]; //Sort distances values and take the median; there's generally a lot of noise and this is the best quick/simple way we had to filter it
        //memcpy(&tmp[0], &ampData[0] , NUMBER_OF_SAMPLES*sizeof(uint32_t));
        //qsort(tmp, NUMBER_OF_SAMPLES, sizeof(uint32_t), uint32Compare);

        tft.setCursor(0, 222);
        tft.setTextColor(HX8357_BLACK);
        tft.print(messageData);
        tft.setCursor(0, 222);
        tft.setTextColor(HX8357_WHITE);
        //messageData = tmp[NUMBER_OF_SAMPLES/2];
        messageData = 0;
        for (unsigned int i : ampData) {
            messageData += i;
        }
        messageData/=NUMBER_OF_SAMPLES;
        messageData*=10;
        if (parameterList[intMaxForce] && messageData > parameterList[intMaxForce]){
            TIMSK3 &= (0 << OCIE3A);
            auto lastTime = (uint32_t) millis();
            while(true){
                tft.setCursor(0, 222);
                tft.setTextColor(HX8357_RED);
                tft.print(messageData);
                while(((uint32_t) millis() - lastTime) < 2000){
                    int8_t keyValue = checkKeypad();
                    if (keyValue >= 0){
                        return 1;
                    }
                }
                lastTime = (uint32_t) millis();
                tft.setCursor(0, 222);
                tft.setTextColor(HX8357_BLACK);
                tft.print(messageData);
                while(((uint32_t) millis() - lastTime) < 2000) {
                    int8_t keyValue = checkKeypad();
                    if (keyValue >= 0) {
                        return 1;
                    }
                }
                lastTime = (uint32_t) millis();

            }
        }


        tft.print(messageData);

        lastDelay = (uint32_t) millis();
        while((millis() - lastDelay) <= (parameterList[intDelay] * 1000)){
        }
        delayActive = 0;
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
            if (counter < (NUMBER_OF_SAMPLES/2)){
                writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "r0x0c", (char * ) nullptr);
                writerSendMessage(&outputMessage);
                counter++;
            }
        }
        //Serial.println("delay");
        delayActive = 1;
        lastDelay = (uint32_t) millis();
        while((millis() - lastDelay) <= (parameterList[intDelay] * 1000)){

        }
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_BLACK);
        tft.print(currentIteration);
        tft.setCursor(0, 134);
        tft.setTextColor(HX8357_WHITE);
        currentIteration++;
        tft.print(currentIteration);
        delayActive = 0;
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
    tft.print("Last Cycle:");
    tft.println(storageReadLine("SYSTEM_1.VAR", 1).toInt() + 1);
    tft.print("Last Run Time:");
    tft.println(storageReadLine("SYSTEM_1.VAR", 2).toInt());

    while(true){
        int8_t keyValue = checkKeypad();
        if (keyValue >= 0){
            break;
        }
    }

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
    // PORTA |= (1 << PA1);
    //Serial.println(degree);

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
    // PORTA &= ~(1 << PA1);

    return 1;
}

uint8_t processHome(){
    PORTA &= ~(1 << PA0);
    PORTA |= (1 << PA1);

    TIMSK3 |= (1 << OCIE3A);

    message_output_t outputMessage{};
    writerPrepMessage(&outputMessage, '\0', '\0', 'i', (char *) "r0", (char *) "0x8000");
    writerSendMessage(&outputMessage);

    delay(100);

    while(!(PINA & (1 << PA2))){

    }
    //PORTA |= (1 << PA0);
    PORTA &= ~(1 << PA1);

    writerPrepMessage(&outputMessage, '\0', '\0', 'i', (char *) "r0", (char *) "0x0000");
    writerSendMessage(&outputMessage);

    TIMSK3 &= (0 << OCIE3A);

    return 1;
}


ISR(TIMER3_COMPA_vect) {
    interuptCounter++;
    /*if(!(interuptCounter%=8) && !delayActive) {
        message_output_t outputMessage{};
        writerPrepMessage(&outputMessage, '\0', '\0', 'g', (char *) "r0x0c", (char *) nullptr);
        writerSendMessage(&outputMessage);
    }*/

    if(read_message(&processMessage)){
        if (processMessage.data.commandCode[0] == 'v'){
            int i = 0;
            interuptCounter2++;
            interuptCounter2%=NUMBER_OF_SAMPLES;
            ampData[interuptCounter2] = 0;
            while (i < processMessage.data.commandParamLength){
                //Serial.print("b ");
                //Serial.print(processMessage.data.commandParam[i]);
                if(processMessage.data.commandParam[i] == 45) {
                    i++;
                    continue;
                }
                ampData[interuptCounter2] = ampData[interuptCounter2] * 10 + (processMessage.data.commandParam[i] - 48);
                //Serial.print("A: ");
                //Serial.println(ampData[interuptCounter2]);
                //Serial.print("F: ");
                //Serial.println(messageData);
                i++;
            }
        }
        //messageCount++;
        message_processed(&processMessage);
    }
}

