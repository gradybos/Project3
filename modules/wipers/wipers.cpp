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

//=====[Declaration of private defines]========================================

#define NUM_INT_SPEEDS 3
#define WIPER_PER_MS_30RPM 3700
#define WIPER_PER_MS_40RPM 2800
#define CONT_PERIOD_S 0.0015
#define DUTY_MIN 0.025
#define DUTY_MID 0.075
#define DUTY_MAX 0.125

//=====[Declaration and initialization of private global variables]============

static wiperState_t wiperState;
static bool wipersRising;

static int intDelays[NUM_INT_SPEEDS] = {8000, 6000, 3000};

static int selectedIntDelay;

static int accumulatedWiperTime;
static int accumulatedIntDelayTime;

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
void intSelectorUpdate();

//=====[Implementations of public functions]===================================

void wipersInit() {
    wipers.period(CONT_PERIOD_S);
    wipersRising = true;
    wiperState = WIPERS_OFF;
    selectedIntDelay = intDelays[0];
}

void wipersUpdate(int systemUpdateTime) {
    wipers.write(DUTY_MID);
    // wiperSelectorUpdate();
    // switch (wiperState) {
    //     case WIPERS_LO:
    //         wipersLo(systemUpdateTime);
    //         break;
    //     case WIPERS_HI:
    //         wipersHi(systemUpdateTime);
    //         break;
    //     case WIPERS_INT:
    //         intSelectorUpdate();
    //         wipersInt(systemUpdateTime);
    //         break;
    //     case WIPERS_OFF:
    //     default:
    //         wipersOff(systemUpdateTime);
    // }
}

wiperState_t wipersRead() {
    return wiperState;
}

//=====[Implementations of private functions]===================================

void wipersOff(int systemDelay) {
    if (accumulatedWiperTime > 0) {
        wipers.write(DUTY_MIN);
        accumulatedWiperTime -= systemDelay;
    }
    else {
        wipersRising = true;
        wipers.write(DUTY_MID);
    }
}

void wipersInt(int systemDelay) { // NOT COMPLETE
    accumulatedIntDelayTime += systemDelay;
    if (accumulatedIntDelayTime > selectedIntDelay) {
        wipersLo(systemDelay);
    }
}

void wipersLo(int systemDelay) {
    accumulatedWiperTime += systemDelay;
    if (accumulatedWiperTime < WIPER_PER_MS_30RPM) {
        wipers.write(DUTY_MAX);
    }
    else {
        wipersRising = false;
    }
}

void wipersHi(int systemDelay) {
    accumulatedWiperTime += systemDelay;
    if (accumulatedWiperTime < WIPER_PER_MS_40RPM) {
        wipers.write(DUTY_MAX);
    }
    else {
        wipersRising = false;
    }
}

void wiperSelectorUpdate() {
    if (!wipersRising) {
        wiperState = WIPERS_OFF;
    }
    else if (wiperSelect.read() > 0.75 && wiperState == WIPERS_OFF) {
        wiperState = WIPERS_HI;
    }
    else if (0.5 < wiperSelect.read() && wiperSelect.read() < 0.75 && wiperState == WIPERS_OFF) {
        wiperState = WIPERS_LO;
    }
    // else if (0.25 < wiperSelect.read() && wiperSelect.read() < 0.5 && wiperState == WIPERS_OFF) {
    //     wiperState = WIPERS_INT;
    // }
    else {
        wiperState = WIPERS_OFF;
    }
}

void intSelectorUpdate() {
    // placeholder
}