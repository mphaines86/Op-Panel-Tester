#include "storage.h"
#include "SD.h"

uint8_t storage_SD_loaded = 0;
String currentWorkingFile = "";

uint8_t storageBeginSD(){
    if (!SD.begin(8))
        return 0;
    return 1;
}

static String readLine(File dataFile, uint16_t lineNumber){
    uint16_t currentLine = 0;
    String line = "";
    do {
        line = dataFile.readStringUntil('\n');
        currentLine++;
    } while(currentLine != lineNumber);
    return line;
}

static void writeIntLine(File dataFile, uint16_t lineNumber, uint8_t data){
    uint16_t currentLine = 0;
    String line = "";
    while(currentLine != lineNumber) {
        line = dataFile.readStringUntil('\n');
        currentLine++;
    }
    char temp[4];
    sprintf(temp,"%4d", data);
    dataFile.write(temp);
}

void storageNewFile(){
    currentWorkingFile = "";
    for (uint16_t &i : parameterList) {
        i =0;
    }
    for (uint8_t &i : booleanList) {
        i = 0;
    }
}

uint8_t storageLoadSD(const String &fileName){
    if(SD.exists(fileName)){
        File dataFile = SD.open(fileName);
        for (uint16_t &i : parameterList){
            i = (uint16_t) dataFile.readStringUntil('\n').toInt();
        }
        for (uint8_t &i : booleanList) {
            i = (uint8_t) dataFile.readStringUntil('\n').toInt();
        }
        dataFile.close();
    }
    return 1;
}

uint8_t storageSaveParameters(){
    if(storage_SD_loaded == 0u) {
        if(storageBeginSD() == 0u);
            return 0;
        storage_SD_loaded = 1;
    }

    char temp[8];

    if(currentWorkingFile == ""){
        currentWorkingFile.toCharArray(temp,8);
    }

    uint8_t current_file_number = 0;
    String lineData;
    if(SD.exists("system_var")){
        File dataFile = SD.open("system_var", FILE_WRITE);
        lineData = readLine(dataFile, 0);
        current_file_number = (uint8_t) lineData.toInt();
        writeIntLine(dataFile, 0, ++current_file_number);
        dataFile.close();
    }
    sprintf(temp, "%08d", current_file_number);

    if(SD.exists(temp)) {
        File dataFile = SD.open(temp);
        for (uint16_t i : parameterList) {
            dataFile.write(i);
            dataFile.write('\n');
        }
        for (uint8_t i : booleanList) {
            dataFile.write(i);
            dataFile.write('\n');
        }
        dataFile.close();
    }
    return 1;
}

String* storageGetFiles(){
    File dir = SD.open("/");
    while (true) {
        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            break;
        }
        Serial.print(entry.name());
        if (!entry.isDirectory()) {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}
