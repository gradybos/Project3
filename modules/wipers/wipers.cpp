//=====[Libraries]=============================================================

#include "ignition.h"
#include "mbed.h"
#include "arm_book_lib.h"
#include "wipers.h"
#include "display.h"

//=====[Declaration of private defines]========================================

#define NUM_INT_SPEEDS 3
#define POS_PERIOD_S 0.02
#define DUTY_MIN 0.025
#define DUTY_INCREMENT 0.0033
#define DUTY_67 0.058

#define SHORT_INT_DELAY 3000
#define MEDIUM_INT_DELAY 6000
#define LONG_INT_DELAY 8000

#define POT_SELECT_PADDING 0.1

//=====[Declaration and initialization of private global variables]============

typedef enum {
    RESTING,
    RISING,
    FALLING,
} servoState_t;

wiperState_t wiperState;
char * stateStr = new char[4];
servoState_t servoState;

int intDelays[NUM_INT_SPEEDS] = {LONG_INT_DELAY, MEDIUM_INT_DELAY, SHORT_INT_DELAY};
char * intDelayStr = new char[6];

int prevIntDelay;
int selectedIntDelay;
float intSelectorThresholds[NUM_INT_SPEEDS-1];

int updateTime;
int accumulatedWiperTime;
int accumulatedIntDelayTime;

float currentDuty;

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

void wipersUpdate() {
    static int servoPeriodUpdate;
    servoPeriodUpdate += updateTime;
    wiperSelectorUpdate();
    if (servoPeriodUpdate >= 20) {
        servoPeriodUpdate = 0;
        switch (wiperState) {
            case WIPERS_LO:
                wipersLo();
                break;
            case WIPERS_HI:
                wipersHi();
                break;
            case WIPERS_INT:
                prevIntDelay = selectedIntDelay;
                selectedIntDelay = intSelectorUpdate();
                if (selectedIntDelay != prevIntDelay) {
                    showingDisplayUpdate();
                }
                wipersInt();
                break;
            case WIPERS_OFF:
            default:
                wipersOff();
        }
    }
    
}

const char * wipersReadState() {
    switch(wiperState) {
        case WIPERS_OFF:
            stateStr[0] = 'O';
            stateStr[1] = 'F';
            stateStr[2] = 'F';
            stateStr[3] = ' ';
            break;
        case WIPERS_INT:
            stateStr[0] = 'I';
            stateStr[1] = 'N';
            stateStr[2] = 'T';
            stateStr[3] = ' ';
            break;
        case WIPERS_LO:
            stateStr[0] = 'L';
            stateStr[1] = 'O';
            stateStr[2] = 'W';
            stateStr[3] = ' ';
            break;
        case WIPERS_HI:
            stateStr[0] = 'H';
            stateStr[1] = 'I';
            stateStr[2] = 'G';
            stateStr[3] = 'H';
            break;
    }
    return stateStr;
}

const char * wipersReadInt() {
    switch(selectedIntDelay) {
        case SHORT_INT_DELAY:
            intDelayStr[0] = 'S';
            intDelayStr[1] = 'H';
            intDelayStr[2] = 'O';
            intDelayStr[3] = 'R';
            intDelayStr[4] = 'T';
            intDelayStr[5] = ' ';
            break;
        case MEDIUM_INT_DELAY:
            intDelayStr[0] = 'M';
            intDelayStr[1] = 'E';
            intDelayStr[2] = 'D';
            intDelayStr[3] = 'I';
            intDelayStr[4] = 'U';
            intDelayStr[5] = 'M';
            break;
        case LONG_INT_DELAY:
            intDelayStr[0] = 'L';
            intDelayStr[1] = 'O';
            intDelayStr[2] = 'N';
            intDelayStr[3] = 'G';
            intDelayStr[4] = ' ';
            intDelayStr[5] = ' ';
            break;
    }
    return intDelayStr;
}

//=====[Implementations of private functions]===================================

void wipersOff() {
    wipers.write(currentDuty);
    if (currentDuty > DUTY_MIN) {
        currentDuty -= DUTY_INCREMENT;
    }
    else {
        servoState = RESTING;
    }
}

void wipersInt() {
    
    wipers.write(currentDuty);
    if (accumulatedIntDelayTime != 0 && servoState == RESTING) {
        wipersOff();
    }
    else if (accumulatedIntDelayTime == 0) {
        wipersLo();
    }
    else {
        wipersLo();
    }

    accumulatedIntDelayTime += 20;

    if (accumulatedIntDelayTime > selectedIntDelay) {
        accumulatedIntDelayTime = 0;
    }
}

void wipersLo() {
    wipers.write(currentDuty);
    switch (servoState) {
        case RESTING:
            servoState = RISING;
            break;
        case RISING:
            currentDuty += DUTY_INCREMENT;
            if (currentDuty >= DUTY_67) {
                servoState = FALLING;
            }
            break;
        case FALLING:
            currentDuty -= DUTY_INCREMENT;
            if (currentDuty <= DUTY_MIN) {
                servoState = RESTING;
            }
            break;
    }
}

void wipersHi() {
    wipers.write(currentDuty);
    switch (servoState) {
        case RESTING:
            servoState = RISING;
            break;
        case RISING:
            currentDuty += DUTY_INCREMENT;
            if (currentDuty >= DUTY_67) {
                servoState = FALLING;
            }
            break;
        case FALLING:
            currentDuty -= DUTY_INCREMENT;
            if (currentDuty <= DUTY_MIN) {
                servoState = RESTING;
            }
            break;
    }
}

void wiperSelectorUpdate() {
    float reading = wiperSelect.read();

    if (!ignitionRead()) {
        if (wiperState != WIPERS_OFF) {
            wiperState = WIPERS_OFF;
            showingDisplayUpdate();
        }
        wiperState = WIPERS_OFF;
    }

    else if ( 0.75 + POT_SELECT_PADDING/2 < reading && wiperState != WIPERS_HI) {
        wiperState = WIPERS_HI;
        showingDisplayUpdate();
    }

    else if ( 0.50 + POT_SELECT_PADDING/2 < reading && reading < 0.75 - POT_SELECT_PADDING/2 && wiperState != WIPERS_LO) {
        wiperState = WIPERS_LO;
        showingDisplayUpdate();
            
    }

    else if ( 0.25 + POT_SELECT_PADDING/2 < reading && reading  < 0.50 - POT_SELECT_PADDING/2 && wiperState != WIPERS_INT) {
        wiperState = WIPERS_INT;
        showingDisplayUpdate();
    }

    else if (reading < 0.25 - POT_SELECT_PADDING/2 && wiperState != WIPERS_OFF) {
        wiperState = WIPERS_OFF;
        showingDisplayUpdate();
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