#include "interface.h"
#include "storage.h"
#include "process.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <SPI.h>
#pragma GCC diagnostic pop

#include "../lib/Adafruit_GFX/Adafruit_GFX.h"
#include "../lib/Adafruit_HX8357/Adafruit_HX8357.h"
#include "../lib/Adafruit_GFX/Fonts/FreeSans9pt7b.h"

#define DEBOUNCE_MAX 4
// These are 'flexible' lines that can be changed
#define TFT_CS 48
#define TFT_DC 46
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

const uint16_t colorBar[] = {0x5ACA, 0x52A9, 0x4A48, 0x4228, 0x4207, 0x39E7};
uint8_t booleanList[boolCount];
uint16_t parameterList[intCount];

enum interfacePage_e {
    mainMenu = 0, STP, CT, S
};

enum interfaceKeypadButton_e {
    kbA = 0,
    kb3,
    kb2,
    kb1,
    kbB,
    kb6,
    kb5,
    kb4,
    kbC,
    kb9,
    kb8,
    kb7,
    kbD,
    kbPound,
    kb0,
    kbAsterisk
};

enum interfaceParamType_e {
    ptMenu = 0, ptParam, ptAction, ptBool, ptNone
};

struct interfaceParam_s {
    enum interfaceParamType_e type;
    int8_t number;
    const char *name;
};

typedef uint8_t (*functionPtr_t)();

//TODO: Remove function pointers
struct interfaceAct_s {
    actionParameter_t type;
    const char *text;
    functionPtr_t actionFunction;
};

functionPtr_t actionFunctionList[6] = {&processCalibrate, &processRun,
                                       &processAttributes, &processHelp,
                                       &processSave, &processLoad};

const struct interfaceParam_s interfaceParameters[5][7] = {
        {
                {ptNone, -1, "Main Menu"},
                {ptMenu,   1,           "A.) Set Testing Parameters"},
                {ptMenu,   2,           "B.) Force Setup & Calibration"},
                {ptAction, actTest,     "C.) Begin Test"},
                {ptMenu,  3,        "D.) Storage"},
                {ptMenu,   4,      "#.) Settings"},
                {ptNone, -1, ""},
        },
        {
                {ptNone, -1, "Set Testing Parameters"},
                {ptParam,  intSpeed,    "A.) Set Speed (0 - 100)"},
                {ptParam,  intMaxAngle, "B.) Max Angle (0 - 90)"},
                {ptParam,  intMinAngle, "C.) Min Angle (0 - 90)"},
                {ptParam, intCycle, "D.) Number of Cycles"},
                {ptMenu,   2,      "#.) Force Setup & Calibration"},
                {ptMenu, 0,  "*.) Main Menu"},
        },
        {
                {ptNone, -1, "Force Setup & Calibration"},
                {ptParam,  intMaxForce, "A.) Max Force (0 - 30)"},
                {ptParam,  intMinForce, "B.) Min Force (0 - 30)"},
                {ptBool,   boolFail,    "C.) Force Out of Bounds Error"},
                {ptParam, intError, "D.)Out of Bounds Error(0-100)"},
                {ptAction, actCal, "#.) Begin Calibration"},
                {ptMenu, 0,  "*.) Main Menu"},
        },
        {
                {ptNone, -1, "Storage"},
                {ptAction, actSave,     "A.) Save Printer Config"},
                {ptAction, actLoad,     "B.) Load Printer Config"},
                {ptParam,  intStore,    "C.) Store test data every\n(0-1000) minutes"},
                {ptMenu,  0,        "D.) Main Menu"},
                {ptNone,   -1,     ""},
                {ptNone, -1, ""},
        },
        {
                {ptNone, -1, "Settings"},
                {ptAction, actAtt,      "A.) Attributes"},
                {ptAction, actHelp,     "B.) Help"},
                {ptMenu,   0,           "C.) Main Menu"},
                {ptNone,  -1,       ""},
                {ptNone,   -1,     ""},
                {ptNone, -1, ""},
        }
};

//TODO: Remove Fuction Pointers
const struct interfaceAct_s interfaceActions[6] = {
        {actCal,  "Calibration of the force\nsensors will now begin. Make sure the area is clear for\ncalibration.", &processCalibrate},
        {actTest, "Testing will now Begin.\nPlease make sure the area is clear for testing.",                        &processRun},
        {actAtt,  "",                                                                                                &processAttributes},
        {actHelp, "",                                                                                                &processHelp},
        {actSave, "Save Testing Parameters?",                                                                                                &processSave},
        {actLoad, "",                                                                                                &processLoad}
};

static struct {
};

static struct {
    uint8_t buttonRows[NUM_OF_ROWS];
    volatile uint8_t *rowsPortRegisters[NUM_OF_ROWS];
    uint8_t buttonColumns[NUM_OF_COLUMNS];
    volatile uint8_t *columnsPortRegisters[NUM_OF_COLUMNS];
    uint8_t currentRow;

    uint8_t integrator[NUM_OF_ROWS][NUM_OF_COLUMNS];
    uint8_t lastButtonPress[NUM_OF_ROWS][NUM_OF_COLUMNS];
    enum interfacePage_e activePage;
    enum interfaceParamType_e activeMenu;
    int8_t workingParameterNumber;
    int8_t sourceNumber;
    uint8_t tempValue;

} interface;

static uint8_t debounce(uint8_t portRegister, uint8_t port, uint8_t row,
                        uint8_t column) {
    //cool++;
    uint8_t output = 0;

    if (!(boolean) (portRegister & (1 << port))) {
        if (interface.integrator[row][column] > 0)
            interface.integrator[row][column]--;
    } else if (interface.integrator[row][column] < DEBOUNCE_MAX)
        interface.integrator[row][column]++;

    if (interface.integrator[row][column] == 0)
        output = 0;
    else if (interface.integrator[row][column] >= DEBOUNCE_MAX) {
        output = 1;
        interface.integrator[row][column] = DEBOUNCE_MAX;
    }

    return output;
}

static void drawMenu() {

    interface.activeMenu = ptMenu;
    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println(interfaceParameters[interface.activePage][0].name);
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    for (int i = 1; i < 7; i++)
        tft.println(interfaceParameters[interface.activePage][i].name);
}


static void drawParamMenu() {
    for (int i = 8; i <= 13; i++)
        tft.fillRect(60, i * 6, 340, 8, colorBar[i - 8]);
    tft.fillRect(60, 86, 340, 130, 0xBE15);
    tft.setTextColor(0x0000);
    tft.setCursor(62, 116);
    tft.print("Current Value: ");
    tft.println(parameterList[interface.workingParameterNumber]);
    tft.setCursor(62, 144);
    tft.println("New Value: ");
    tft.setCursor(62, 200);
    tft.println("A.) Okay  B.) Cancel");
    tft.setCursor(304, 144);


}

static void drawActionMenu() {
    tft.fillScreen(0x2924);
    for (int i = 0; i <= 5; i++)
        tft.fillRect(0, i * 6, tft.width(), 8, colorBar[i]);
    tft.setCursor(0, 24);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(2);
    tft.println();
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    tft.println(interfaceActions[interface.workingParameterNumber].text);
    tft.setTextSize(1);
    tft.println();
    tft.setTextSize(2);
    tft.print("A.) Okay        B.) Cancel");

}

static void drawBoolMenu() {
    for (int i = 8; i <= 13; i++)
        tft.fillRect(60, i * 6, 340, 8, colorBar[i - 8]);
    tft.fillRect(60, 86, 340, 130, 0xBE15);
    tft.setTextColor(0x0000);
    tft.setCursor(62, 116);
    tft.print("Current Value: ");
    if ((boolean) booleanList[interface.workingParameterNumber]) {
        tft.println("Yes");
    } else {
        tft.println("No");
    }
    tft.setCursor(62, 150);
    tft.println("A.) Yes");
    tft.setCursor(62, 178);
    tft.println("B.) No");
}

void interfaceInit() {
    static uint8_t temporary1[] = {PL2, PL4, PL6, PG0};
    memcpy(interface.buttonRows, temporary1, sizeof temporary1);

    static uint8_t temporary2[] = {PC4, PC2, PC0, PG2};
    memcpy(interface.buttonColumns, temporary2, sizeof temporary2);

    static volatile uint8_t *temporary3[] = {&PORTL, &PORTL, &PORTL, &PORTG};
    memcpy(interface.rowsPortRegisters, temporary3, sizeof temporary3);

    static volatile uint8_t *temporary4[] = {&PINC, &PINC, &PINC, &PING};
    memcpy(interface.columnsPortRegisters, temporary4, sizeof temporary4);

    interface.currentRow = 0;
    for (uint8_t i = 0; i < NUM_OF_ROWS; i++) {
        *interface.rowsPortRegisters[i] |= (1 << interface.buttonRows[i]);
        for (uint8_t j = 0; j < NUM_OF_COLUMNS; j++) {
            interface.integrator[i][j] = DEBOUNCE_MAX;
        }
    }

    for (uint16_t &i : parameterList) {
        i = 0;
    }
    //*interface.rowsPortRegisters[0] = (1 << interface.buttonRows[0]);
    tft.begin(HX8357D);
    tft.setFont(&FreeSans9pt7b);
    tft.setRotation(3);
    interface.activePage = mainMenu;
    drawMenu();

}

void handleActionInput() {
    if ((boolean) interfaceActions[interface.workingParameterNumber].text) {
        if (interface.sourceNumber == 10) {
            //if((*interfaceActions.actionFunction[interface.workingParameterNumber])())
            if ((boolean) (*actionFunctionList[interface.workingParameterNumber])())
                drawMenu();
        } else if (interface.sourceNumber == 11) {
            drawMenu();
        }
    } else {
        drawMenu();
    }
}

void handleBoolInput() {
    if (interface.sourceNumber == 10) {
        booleanList[interface.workingParameterNumber] = 1;
        drawMenu();
    } else if (interface.sourceNumber == 11) {
        booleanList[interface.workingParameterNumber] = 0;
        drawMenu();
    }
}

void handleParamInput() {
    if (interface.sourceNumber >= 10) {
        if (interface.sourceNumber == 10) {
            parameterList[interface.workingParameterNumber] = interface.tempValue;
            interface.tempValue = 0;
            drawMenu();
        } else if (interface.sourceNumber == 11) {
            interface.tempValue = 0;
            drawMenu();
        }
        return;
    }
    if (interface.sourceNumber > -1) {
        tft.print(interface.sourceNumber);
        interface.tempValue = interface.tempValue * 10 + interface.sourceNumber;
    }
}


static void handleMenuInput(const struct interfaceParam_s *inter) {
    interface.workingParameterNumber = inter->number;
    interface.activeMenu = inter->type;
    switch (inter->type) {
        case ptMenu:
            interface.activePage = (interfacePage_e) inter->number;
            drawMenu();
            return;
        case ptParam:
            drawParamMenu();
            return;
        case ptBool:
            drawBoolMenu();
            return;
        case ptAction:
            drawActionMenu();
            return;
        case ptNone:
            return;

    }
}

static void handleUserInput(int16_t source) {
    switch (source) {
        case kb1:
            interface.sourceNumber = 1;
            return;
        case kb2:
            interface.sourceNumber = 2;
            return;
        case kb3:
            interface.sourceNumber = 3;
            return;
        case kb4:
            interface.sourceNumber = 4;
            return;
        case kb5:
            interface.sourceNumber = 5;
            return;
        case kb6:
            interface.sourceNumber = 6;
            return;
        case kb7:
            interface.sourceNumber = 7;
            return;
        case kb8:
            interface.sourceNumber = 8;
            return;
        case kb9:
            interface.sourceNumber = 9;
            return;
        case kb0:
            interface.sourceNumber = 0;
            return;
        case kbA:
            interface.sourceNumber = 10;
            return;
        case kbB:
            interface.sourceNumber = 11;
            return;
        case kbC:
            interface.sourceNumber = 12;
            return;
        case kbD:
            interface.sourceNumber = 13;
            return;
        case kbPound:
            interface.sourceNumber = 14;
            return;
        case kbAsterisk:
            interface.sourceNumber = 15;
            return;
        default:
            return;

    }


}

void checkKeypad() {
    *interface.rowsPortRegisters[interface.currentRow] &= ~(1
            << interface.buttonRows[interface.currentRow]);
    uint8_t i;
    for (i = 0; i < NUM_OF_COLUMNS; i++) {
        uint8_t output = debounce(*interface.columnsPortRegisters[i],
                                  interface.buttonColumns[i],
                                  interface.currentRow,
                                  i);
        if (!(boolean) output &&
            !(boolean) interface.lastButtonPress[interface.currentRow][i]) {
            interface.lastButtonPress[interface.currentRow][i] = 1;
            //Serial.print("Button Pressed: ");
            //Serial.println((interface.currentRow * NUM_OF_ROWS) + i);
            handleUserInput((interface.currentRow * NUM_OF_ROWS) + i);

        } else if ((boolean) output) {
            interface.lastButtonPress[interface.currentRow][i] = 0;
        }
    }
    *interface.rowsPortRegisters[interface.currentRow] |= (1
            << interface.buttonRows[interface.currentRow]);
    ++interface.currentRow %= (NUM_OF_ROWS);
}

void interfaceCheck() {
    checkKeypad();
    if (interface.sourceNumber > -1) {
        switch (interface.activeMenu) {
            case ptMenu:
                if (interface.sourceNumber >= 10)
                    handleMenuInput(&interfaceParameters[interface.activePage][
                            interface.sourceNumber - 9]);
                break;
            case ptParam:
                handleParamInput();
                break;
            case ptBool:
                handleBoolInput();
                break;
            case ptAction:
                handleActionInput();
                break;
            case ptNone:
                break;
        }
        interface.sourceNumber = -1;
    }
}
