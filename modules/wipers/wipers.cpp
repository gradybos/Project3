/*

Should have the following
    -Potentiometer selects between 4 modes
        -Hi, Lo, Int, Off
        -Hi => 0-67 degrees at 40 rpm
        -Lo => 0-67 degrees at 30 rpm
        -Int => Same params as low, add delays between "wipes"
            -Should wait 3, 6, or 8 seconds depending on potentiometer input
    -Read wiper setting (for display)

*/

//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "wipers.h"
#include "display.h"

//=====[Declaration of private defines]========================================

#define NUM_INT_SPEEDS 3
#define POS_PERIOD_S 0.02
#define DUTY_MIN 0.025
#define DUTY_67 0.058
#define WIPE_TIME_LO 700
#define WIPE_TIME_HI 500

//=====[Declaration and initialization of private global variables]============

wiperState_t wiperState;
bool wipersRising;

int intDelays[NUM_INT_SPEEDS] = {8000, 6000, 3000};

int selectedIntDelay;
float intSelectorThresholds[NUM_INT_SPEEDS-1];

int accumulatedWiperTime;
int accumulatedIntDelayTime;

PwmOut wipers(PF_9);
AnalogIn wiperSelect(A0);
AnalogIn intSelect(A1);

//=====[Declaration and initialization of public global objects]===============

//=====[Declaration and initialization of public global variables]=============

//=====[Declarations (prototypes) of private functions]=========================

void wipersOff(int systemDelay);
void wipersInt(int systemDelay);
void wipersLo(int systemDelay);
void wipersHi(int systemDelay);
void wiperSelectorUpdate();
int intSelectorUpdate();
void showingDisplayInit();
void showingDisplayUpdate();

//=====[Implementations of public functions]===================================

void wipersInit() {
    wipers.period(POS_PERIOD_S);
    wipersRising = true;
    wiperState = WIPERS_OFF;
    selectedIntDelay = intDelays[0];
    for (int i=1; i<NUM_INT_SPEEDS ;i++) {
        intSelectorThresholds[i-1] = (1.0*i)/NUM_INT_SPEEDS;
    }
    showingDisplayInit();
}

// display the selected mode, including the delay time selection (SHORT, MEDIUM, or LONG) 
// for intermittent mode on the LCD display
void showingDisplayInit() {
    displayCharPositionWrite(0,0);
    displayStringWrite("Mode: OFF");

    displayCharPositionWrite(0,1);
    displayStringWrite("Int:  OFF");
}

void showingDisplayUpdate() {
    displayCharPositionWrite(0,0);
    displayStringWrite("Mode:");

    displayCharPositionWrite(0,1);
    displayStringWrite("Int:");
}

void wipersUpdate(int systemUpdateTime) {
    selectedIntDelay = intSelectorUpdate();
    wipersInt(systemUpdateTime);

    showingDisplayUpdate();

    wiperSelectorUpdate();
    switch (wiperState) {
        case WIPERS_LO:
            wipersLo(systemUpdateTime);
            break;
        case WIPERS_HI:
            wipersHi(systemUpdateTime);
            // displayStringWrite("HIGH ");
            break;
        case WIPERS_INT:
            intSelectorUpdate();
            wipersInt(systemUpdateTime);
            // displayStringWrite("INT  ");
            break;
        case WIPERS_OFF:
            wipersOff(systemUpdateTime);
            break;
        default:
            wipersOff(systemUpdateTime);
    }
}

wiperState_t wipersRead() {
    return wiperState;
}

//=====[Implementations of private functions]===================================

void wipersOff(int systemDelay) {
    wipers.write(DUTY_MIN);
}

void wipersInt(int systemDelay) {
    accumulatedIntDelayTime += systemDelay;
    if (accumulatedIntDelayTime < WIPE_TIME_LO*2) {
        wipersLo(systemDelay);
    }
    else {
        wipersOff(systemDelay);
    }
    if (accumulatedIntDelayTime > selectedIntDelay) {
        accumulatedIntDelayTime = 0;
    }
}

void wipersLo(int systemDelay) {
    accumulatedWiperTime += systemDelay;
    if (accumulatedWiperTime < WIPE_TIME_LO && wipersRising) {
        wipers.write(DUTY_67);
    }
    else if (accumulatedWiperTime < WIPE_TIME_LO && !wipersRising) {
        wipers.write(DUTY_MIN);
    }
    else {
        accumulatedWiperTime = 0;
        wipersRising = !wipersRising;
    }
}

void wipersHi(int systemDelay) {
    accumulatedWiperTime += systemDelay;
    if (accumulatedWiperTime < WIPE_TIME_HI && wipersRising) {
        wipers.write(DUTY_67);
    }
    else if (accumulatedWiperTime < WIPE_TIME_HI && !wipersRising) {
        wipers.write(DUTY_MIN);
    }
    else {
        accumulatedWiperTime = 0;
        wipersRising = !wipersRising;
    }
}

void wiperSelectorUpdate() {
    if (!wipersRising) {
        wiperState = WIPERS_OFF;
    }
    else if (wiperSelect.read() > 0.75 && wiperState == WIPERS_OFF) {
        displayCharPositionWrite(6,0);
        displayStringWrite("HIGH ");
        wiperState = WIPERS_HI;
    }
    else if (0.5 < wiperSelect.read() && wiperSelect.read() < 0.75 && wiperState == WIPERS_OFF) {
        displayCharPositionWrite(6,0);
        displayStringWrite("LOW  ");
        wiperState = WIPERS_LO;
    }
    else if (0.25 < wiperSelect.read() && wiperSelect.read() < 0.5 && wiperState == WIPERS_OFF) {
        wiperState = WIPERS_INT;
        displayCharPositionWrite(6,0);
        displayStringWrite("INT ");
    }
    else {
        wiperState = WIPERS_OFF;
        displayStringWrite("OFF ");
    }
}

int intSelectorUpdate() {
    float selectorReading = intSelect.read();
    for (int i=0; i<NUM_INT_SPEEDS-1 ; i++) {
        if (selectorReading < intSelectorThresholds[i]) {
            return intDelays[i];
        }
    }
    return intDelays[NUM_INT_SPEEDS-1];
}