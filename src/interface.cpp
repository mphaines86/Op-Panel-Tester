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
#define TFT_CS 2
#define TFT_DC 3
#define TFT_RST 4 // RST can be set to -1 if you tie it to Arduino's reset
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// These are the RGB values for the top bar on each screen
const uint16_t colorBar[] = {0x5ACA, 0x52A9, 0x4A48, 0x4228, 0x4207, 0x39E7};

uint8_t booleanList[boolCount];
uint32_t parameterList[intCount];

// TODO: Delete if never used
enum interfacePage_e {
    mainMenu = 0, STP, CT, S
};

// enum of keyboard buttons based off their keyboard position
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

// Enumeration of each type of interface action
enum interfaceParamType_e {
    ptMenu = 0, ptParam, ptAction, ptBool, ptNone
};


// The structure is used to organize and link various aspects of the menu screen
struct interfaceParam_s {
    enum interfaceParamType_e type;
    int8_t number;
    const char *name;
};

// Used when defining the okay/cancel menu of all actionParameters
struct interfaceAct_s {
    actionParameter_t type;
    const char *text;
};

typedef uint8_t (*functionPtr_t)();

// List of pointers to functions
functionPtr_t actionFunctionList[8] = {&processCalibrate, &processRun,
                                       &processAttributes, &processHelp,
                                       &processSave, &processLoad, &processNew, &processHome};

const struct interfaceParam_s interfaceParameters[3][7] = {
        {
                {ptNone, -1, "Main Menu"},
                {ptMenu,   2,           "A.) Homing and Log"},
                {ptMenu,   1,           "B.) Set Testing Parameters"},
                {ptAction, actTest,     "C.) Begin Test"},
                {ptNone,  -1,        ""},
                {ptNone, -1, ""},
                {ptNone, -1, ""},
        },
        {
                {ptNone, -1, "Set Testing Parameters"},
                {ptParam,  intSpeed,    "A.) Set Speed (0 - 100)"},
                {ptParam,  intMaxAngle, "B.) Max Angle (0 - 180)"},
                {ptParam,  intMinAngle, "C.) Min Angle (0 - 180)"},
                {ptParam,  intCycle,    "D.) Number of Cycles"},
                {ptParam,  intDelay,    "#.) Cycle Delay (0 - 255)"},
                {ptMenu, 0,  "*.) Main Menu"},
        },
        {
                {ptNone, -1, "Homing and Log"},
                {ptAction, actHome,     "A.) Home Arm"},
                {ptAction,  actHelp, "B.) Log"},
                {ptParam,   intMaxForce, "C.) Set Max Amperage"},
                {ptBool, boolMove, "D.) Move when Setting Angle"},
                {ptMenu, 0, "#.) Main Menu"},
                {ptNone, -1,  ""},
        },
        /*{
                {ptNone, -1, "Storage"},
                {ptAction, actSave,     "A.) Save Printer Config"},
                {ptAction, actLoad,     "B.) Load Printer Config"},
                {ptAction, actNew,      "C.) New Printer Config"},
                {ptParam,  intStore,    "D.) Store test data every\n(0-1000) cycles"},
                {ptMenu,   0,           "#.) Main Menu"},
                {ptNone, -1, ""},
        }*/
};

const struct interfaceAct_s interfaceActions[8] = {
        {actCal,  "Calibration of the force\nsensors will now begin. Make sure the area is clear for\ncalibration." },
        {actTest, "Testing will now Begin.\nPlease make sure the area is clear for testing. Arm will\nmove to initial position"},
        {actAtt,  ""                                                                                                },
        {actHelp, ""                                                                                                },
        {actSave, "Save Testing Parameters?"                                                                        },
        {actLoad, ""                                                                                                },
        {actNew,  "A new set of parameters \nwill be initilized. All \nunsaved data will be lost."                  },
        {actHome, "The device will now be \nhomed. Please make sure \nthe area is clear."                           }
};

// Defining a struct to organize interface values
static struct {
    uint8_t buttonRows[NUM_OF_ROWS]; // Keeps track of what pin on the MC the keypad row correlates to
    volatile uint8_t *rowsPortRegisters[NUM_OF_ROWS]; // keeps track of what port on the MC is correlated to each row
    uint8_t buttonColumns[NUM_OF_COLUMNS]; // Keeps track of current keypad column
    volatile uint8_t *columnsPortRegisters[NUM_OF_COLUMNS]; // Keeps track of what pin on the MC the keypad column correlates to
    uint8_t currentRow; // Keeps track of the current row we are looking to see if there has been input

    uint8_t integrator[NUM_OF_ROWS][NUM_OF_COLUMNS]; // used for the debounce routine
    uint8_t lastButtonPress[NUM_OF_ROWS][NUM_OF_COLUMNS]; // Keeps track of the last button the was pressed
    enum interfacePage_e activePage;
    enum interfaceParamType_e activeMenu; // Used to determine which page of the menu screen we are in
    int8_t workingParameterNumber; // Determines which parameter has been selected from parameterList or booleanList in storage.h
    int8_t sourceNumber; // Used to determine the current active secondary index of interfaceParameters
    uint32_t tempValue;

} interface;

// Used to avoid a "bounce" of the input from the keypad
static uint8_t debounce(uint8_t portRegister, uint8_t port, uint8_t row,
                        uint8_t column) {

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

// General function used to draw a menu
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

// Draws the menu used for inputting parameter values
static void drawParamMenu() {
    for (int i = 8; i <= 13; i++)
        tft.fillRect(60, i * 6, 340, 8, colorBar[i - 8]);
    tft.fillRect(60, 86, 340, 130, 0xBE15);
    tft.setTextColor(0x0000);
    tft.setCursor(62, 116);
    tft.print("Set Value: ");
    tft.println(parameterList[interface.workingParameterNumber]);
    tft.setCursor(62, 144);
    tft.println("New Value: ");
    tft.setCursor(62, 200);
    tft.println("A.) Okay  B.) Cancel");
    tft.setCursor(253, 144);



}

// Draws the Okay/Cancel menu for actions
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

// Draws the menu for yes/no parameters
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

// Initiate the interface
void interfaceInit() {

    // TODO: Remove if never used
    processBegin();

    // Initialize all of the variable that need to point the pins and registers on the MC
    static uint8_t temporary1[] = {PL2, PL4, PL6, PG0};
    memcpy(interface.buttonRows, temporary1, sizeof temporary1);

    static uint8_t temporary2[] = {PC4, PC2, PC0, PG2};
    memcpy(interface.buttonColumns, temporary2, sizeof temporary2);

    static volatile uint8_t *temporary3[] = {&PORTL, &PORTL, &PORTL, &PORTG};
    memcpy(interface.rowsPortRegisters, temporary3, sizeof temporary3);

    static volatile uint8_t *temporary4[] = {&PINC, &PINC, &PINC, &PING};
    memcpy(interface.columnsPortRegisters, temporary4, sizeof temporary4);

    // Initialize all button pins to the high state and set the integrator so that the debounce responds correctly
    interface.currentRow = 0;
    for (uint8_t i = 0; i < NUM_OF_ROWS; i++) {
        *interface.rowsPortRegisters[i] |= (1 << interface.buttonRows[i]);
        for (uint8_t j = 0; j < NUM_OF_COLUMNS; j++) {
            interface.integrator[i][j] = DEBOUNCE_MAX;
        }
    }

    // Initialize all parameters to 0
    for (uint32_t &i : parameterList) {
        i = 0;
    }

    tft.begin(HX8357D); // Initialize the screen
    tft.setFont(&FreeSans9pt7b); // Set the font to be used
    tft.setRotation(1); // rotate the screen into the right position

    // TODO: Check to see if variable is in use
    interface.activePage = mainMenu;
    drawMenu();

    // TODO: Remove these initialized variable before shipping
    parameterList[intMinAngle] = 15;
    parameterList[intMaxAngle] = 90;
    parameterList[intCycle] = 50;
    booleanList[boolMove] = 1;
    parameterList[intStore] = 5;
    parameterList[intSpeed] = 100;

}

// Handles what to do when the user decides to make the tester perform an action
void handleActionInput() {
    // First if checks if interfaceActions has an empty string and if so skip writing the Okay/Cancel menu.
    if (strcmp(interfaceActions[interface.workingParameterNumber].text,"") != 0) {
        if (interface.sourceNumber == 10) { // Check if A has been pressed
            // This last if runs the function and wait for a value of 1 to be returned to continue
            if ((boolean) (*actionFunctionList[interface.workingParameterNumber])())
                drawMenu();
        } else if (interface.sourceNumber == 11) { // Go back to menu if B is pressed
            drawMenu();
        }
    } else { // Immediately run function because the corresponding entry in interfaceActions has no text
        if ((boolean) (*actionFunctionList[interface.workingParameterNumber])())
            drawMenu();
        else
            drawMenu();
    }
}

// Handle what to do during a yes/no entry
void handleBoolInput() {
    if (interface.sourceNumber == 10) { // if A is pressed
        booleanList[interface.workingParameterNumber] = 1;
        drawMenu();
    } else if (interface.sourceNumber == 11) { // if B is pressed
        booleanList[interface.workingParameterNumber] = 0;
        drawMenu();
    }
}

// Handles the entry of parameters
void handleParamInput() {
    //Serial.print("Current Value:");
    //Serial.println(parameterList[interface.workingParameterNumber]);
    if (interface.sourceNumber >= 10) { // Did the user press A, B, C, D, #, or, *
        //Serial.println();
        if (interface.sourceNumber == 10) { // If it was A set the proper variables to the desired value
            parameterList[interface.workingParameterNumber] = interface.tempValue;
            interface.tempValue = 0;

            // To parameters we want to have the arm move if the user puts in an angle
            if (interface.workingParameterNumber == intMaxAngle || interface.workingParameterNumber == intMinAngle){
                if (booleanList[boolMove])
                    processMove(parameterList[interface.workingParameterNumber]);
            }

            drawMenu();
        } else if (interface.sourceNumber == 11) { // If B was pressed do nothing and go back the the menu
            interface.tempValue = 0;
            drawMenu();
        }
        return;
    } //TODO: Check if changing if to else if breaks anything
    else if (interface.sourceNumber > -1) { // Did the user press any numbers
        tft.print(interface.sourceNumber);
        interface.tempValue = interface.tempValue * 10 + interface.sourceNumber;
    }
}

// Handles inputs on the menu screens
static void handleMenuInput(const struct interfaceParam_s *inter) {
    interface.workingParameterNumber = inter->number;
    interface.activeMenu = inter->type;
    // Checks
    switch (inter->type) {
        case ptMenu:
            // TODO: Check to see if variable is in use.
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
            if (strcmp(interfaceActions[interface.workingParameterNumber].text, "") != 0) {
                drawActionMenu();
            }
            else{
                handleActionInput();
            }
            return;
        case ptNone:
            interface.activeMenu = ptMenu;
            return;

    }
}

int8_t handleUserInput(int16_t source) {
    switch (source) {
        case kb1:
            return 1;
        case kb2:
            return 2;
        case kb3:
            return 3;
        case kb4:
            return 4;
        case kb5:
            return 5;
        case kb6:
            return 6;
        case kb7:
            return 7;
        case kb8:
            return 8;
        case kb9:
            return 9;
        case kb0:
            return 0;
        case kbA:
            return 10;
        case kbB:
            return 11;
        case kbC:
            return 12;
        case kbD:
            return 13;
        case kbPound:
            return 14;
        case kbAsterisk:
            return 15;
        default:
            return -1;

    }


}

int8_t checkKeypad() {
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
            return ((interface.currentRow * NUM_OF_ROWS) + i);

        } else if ((boolean) output) {
            interface.lastButtonPress[interface.currentRow][i] = 0;
        }
    }
    *interface.rowsPortRegisters[interface.currentRow] |= (1
            << interface.buttonRows[interface.currentRow]);

    ++interface.currentRow %= (NUM_OF_ROWS);

    return -1;
}

void interfaceCheck() {
    int8_t inputValue = checkKeypad();
    
    if(inputValue != -1)
        interface.sourceNumber = handleUserInput(inputValue);
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
                Serial.println("Do Nothing");
                break;
        }
        interface.sourceNumber = -1;
    }
}
