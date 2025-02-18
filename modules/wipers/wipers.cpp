/*

To-do List
    - Refactor display commands to be within display module (use wipersRead function)
    - Average potentiometer readings or add a delay to prevent bouncing
    - Make System turn on after igintion and off after system is killed
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
#define WIPE_TIME_LO 900
#define WIPE_TIME_HI 500

//=====[Declaration and initialization of private global variables]============

typedef enum {
    RESTING,
    RISING,
    FALLING
} servoState_t;

wiperState_t wiperState;
servoState_t servoState;

int intDelays[NUM_INT_SPEEDS] = {8000, 6000, 3000};

int selectedIntDelay;
float intSelectorThresholds[NUM_INT_SPEEDS-1];

int updateTime;
int accumulatedWiperTime;
int accumulatedIntDelayTime;

PwmOut wipers(PF_9);
AnalogIn wiperSelect(A0);
AnalogIn intSelect(A1);

//=====[Declaration and initialization of public global objects]===============

//=====[Declaration and initialization of public global variables]=============

//=====[Declarations (prototypes) of private functions]=========================

void wipersOff();
void wipersInt();
void wipersLo();
void wipersHi();
void wiperSelectorUpdate();
int intSelectorUpdate();
void showingDisplayUpdate();

//=====[Implementations of public functions]===================================


int getSelectedIntDelay(){
    return selectedIntDelay/1000;
}

void wipersInit(int systemUpdateTime) {
    wipers.period(POS_PERIOD_S);
    wiperState = WIPERS_OFF;
    servoState = FALLING;
    updateTime = systemUpdateTime;
    selectedIntDelay = intDelays[0];
    for (int i=1; i<NUM_INT_SPEEDS ;i++) {
        intSelectorThresholds[i-1] = (1.0*i)/NUM_INT_SPEEDS;
    }
    showingDisplayInit();
}

// display the selected mode, including the delay time selection (SHORT, MEDIUM, or LONG) 
// for intermittent mode on the LCD display


void wipersUpdate() {
    showingDisplayUpdate();
    wiperSelectorUpdate();
    switch (wiperState) {
        case WIPERS_LO:
            displayModeWriteState("LOW ");
            wipersLo();
            break;
        case WIPERS_HI:
            displayModeWriteState("HIGH");
            wipersHi();
            break;
        case WIPERS_INT:
            displayModeWriteState("INT ");
            selectedIntDelay = intSelectorUpdate();
            wipersInt();
            break;
        case WIPERS_OFF:
        default:
            displayModeWriteState("OFF ");
            wipersOff();
    }
}

wiperState_t wipersRead() {
    return wiperState;
}

//=====[Implementations of private functions]===================================

void wipersOff() {
    wipers.write(DUTY_MIN);
    accumulatedWiperTime += updateTime;
    if (accumulatedWiperTime > WIPE_TIME_LO*2) {
        servoState = RESTING;
        accumulatedWiperTime = 0;
    }
}

void wipersInt() {

    accumulatedIntDelayTime += updateTime;
    if (accumulatedIntDelayTime < WIPE_TIME_LO*2) {
        wipersLo();
    }
    else {
        wipersOff();
    }

    if (accumulatedIntDelayTime > selectedIntDelay) {
        accumulatedIntDelayTime = 0;
    }
}

void wipersLo() {
    accumulatedWiperTime += updateTime;
    if (accumulatedWiperTime <= updateTime) {
        servoState = RESTING;
    }
    else if (accumulatedWiperTime < WIPE_TIME_LO) {
        servoState = RISING;
        wipers.write(DUTY_67);
    }
    else if (accumulatedWiperTime >= WIPE_TIME_LO*2) {
        accumulatedWiperTime = 0;
    }
    else if (accumulatedWiperTime >= WIPE_TIME_LO) {
        servoState = FALLING;
        wipers.write(DUTY_MIN);
    }   
}

void wipersHi() {
    accumulatedWiperTime += updateTime;
    if (accumulatedWiperTime <= updateTime) {
        servoState = RESTING;
    }
    else if (accumulatedWiperTime < WIPE_TIME_HI) {
        servoState = RISING;
        wipers.write(DUTY_67);
    }
    else if (accumulatedWiperTime >= WIPE_TIME_HI*2) {
        accumulatedWiperTime = 0;
    }
    else if (accumulatedWiperTime >= WIPE_TIME_HI) {
        servoState = FALLING;
        wipers.write(DUTY_MIN);
    }
}

void wiperSelectorUpdate() {
    float reading = wiperSelect.read();
    if (reading > 0.7 && servoState == RESTING) {
        
        wiperState = WIPERS_HI;
    }
    else if (0.45 < reading && reading < 0.55 && servoState == RESTING) {
        
        wiperState = WIPERS_LO;
    }
    else if (0.2 < reading && reading < 0.3 && servoState == RESTING) {
        wiperState = WIPERS_INT;
        
    }
    else if (reading < 0.10) {
        wiperState = WIPERS_OFF;
        
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