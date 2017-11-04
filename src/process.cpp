#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "storage.h"
#include "interface.h"

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
    message_output_t outputMessage;
    const char *memoryBank = "r0xab";
    const char *commandParam = "0";
    writerPrepMessage(&outputMessage, '\0', '\0', 'r', (char *) memoryBank, '\0');
    //writerPrepMessage(&outputMessage, '\0', '\0', 's', (char *) memoryBank,
    //                  (char *) commandParam);
    writerSendMessage(&outputMessage);
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
    fsrReading = analogRead(fsrPin);
    Serial.print("Analog reading = ");
    Serial.println(fsrReading);

    // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
    fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
    Serial.print("Voltage reading in mV = ");
    Serial.println(fsrVoltage);

    if (fsrVoltage == 0) {
        Serial.println("No pressure");
    } else {
        // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
        // so FSR = ((Vcc - V) * R) / V        yay math!
        fsrResistance = 5000 -
                        fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
        fsrResistance *= 10000;                // 10K resistor
        fsrResistance /= fsrVoltage;
        Serial.print("FSR resistance in ohms = ");
        Serial.println(fsrResistance);

        fsrConductance = 1000000;           // we measure in micromhos so
        fsrConductance /= fsrResistance;
        Serial.print("Conductance in microMhos: ");
        Serial.println(fsrConductance);

        // Use the two FSR guide graphs to approximate the force
        if (fsrConductance <= 1000) {
            fsrForce = fsrConductance / 80 * 0.22481;
            Serial.print("Force in Pounds: ");
            Serial.println(fsrForce);
        } else {
            fsrForce = fsrConductance - 1000;
            fsrForce /= 30;
            Serial.print("Force in Pounds: ");
            Serial.println(fsrForce * 0.22481);
        }
    }
    Serial.println("--------------------");
}
