//=====[Libraries]=============================================================

#include "ignition.h"
#include "mbed.h"
#include "arm_book_lib.h"
#include "wipers.h"
#include "display.h"

//=====[Declaration of private defines]========================================

#define NUM_INT_SPEEDS 3

#define POS_PERIOD_S 0.02
#define POS_PERIOD_MS 20
#define DUTY_MIN 0.025
#define DUTY_INCREMENT_LO 0.001773
#define DUTY_INCREMENT_HI 0.002364
#define DUTY_67 0.058

#define SHORT_INT_DELAY 3000
#define MEDIUM_INT_DELAY 6000
#define LONG_INT_DELAY 8000

#define POT_SELECT_PADDING 0.1

//=====[Declaration and initialization of private datatypes]============

typedef enum {
    RESTING,
    RISING,
    FALLING,
} servoState_t;

//=====[Declaration and initialization of private global variables]============

// Wiper state variables
wiperState_t wiperState;

// Interval delay variables
int intDelays[NUM_INT_SPEEDS] = {LONG_INT_DELAY, MEDIUM_INT_DELAY, SHORT_INT_DELAY};
int prevIntDelay;
int selectedIntDelay;
float intSelectorThresholds[NUM_INT_SPEEDS-1];

// Timing variables
int updateTime;
int accumulatedIntDelayTime;

// Servo variables
servoState_t servoState;
float currentDuty;

PwmOut wipers(PF_9);
AnalogIn wiperSelect(A0);
AnalogIn intSelect(A1);

//=====[Declarations (prototypes) of private functions]=========================

static void wipersOff();
static void wipersInt();
static void wipersLo();
static void wipersHi();
static void wiperSelectorUpdate();
static int intSelectorUpdate();
static const char * intDelayToStr();

//=====[Implementations of public functions]===================================

void wipersInit(int systemUpdateTime) {
    updateTime = systemUpdateTime;
    wipers.period(POS_PERIOD_S);
    wiperState = WIPERS_OFF;
    servoState = FALLING;
    selectedIntDelay = intDelays[0];
    // Create thresholds for interval selector potentiometer
    for (int i=1; i<NUM_INT_SPEEDS ;i++) {
        intSelectorThresholds[i-1] = (1.0*i)/NUM_INT_SPEEDS;
    }
    showingDisplayInit();
}

void wipersUpdate() {
    static int servoPeriodUpdate;
    servoPeriodUpdate += updateTime;
    wiperSelectorUpdate();
    // Make sure servo is only updated each period
    if (servoPeriodUpdate >= POS_PERIOD_MS) {
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
                // Update display only when necessary
                if (selectedIntDelay != prevIntDelay) {
                    displayWriteInt(intDelayToStr());
                }
                wipersInt();
                break;
            case WIPERS_OFF:
            default:
                wipersOff();
        }
    }
    
}

//=====[Implementations of private functions]===================================

static void wipersOff() {
    // Move wipers towards resting position
    wipers.write(currentDuty);
    if (currentDuty > DUTY_MIN) {
        currentDuty -= DUTY_INCREMENT_LO;
    }
    else {
        servoState = RESTING;
    }
}

static void wipersInt() {
    
    wipers.write(currentDuty);
    // This only occurs after a full cycle
    if (accumulatedIntDelayTime != 0 && servoState == RESTING) {
        wipersOff();
    }
    // This moves the wipers a full cycle
    else if (accumulatedIntDelayTime == 0) {
        wipersLo();
    }
    else {
        wipersLo();
    }

    // Allow another full cycle once time elapses
    accumulatedIntDelayTime += POS_PERIOD_MS;

    if (accumulatedIntDelayTime > selectedIntDelay) {
        accumulatedIntDelayTime = 0;
    }
}

static void wipersLo() {
    wipers.write(currentDuty);
    switch (servoState) {
        case RESTING:
            servoState = RISING;
            break;
        case RISING:
            // Move wipers out until they reach 67 degrees
            currentDuty += DUTY_INCREMENT_LO;
            if (currentDuty >= DUTY_67) {
                servoState = FALLING;
            }
            break;
        case FALLING:
            // Then retract them
            currentDuty -= DUTY_INCREMENT_LO;
            if (currentDuty <= DUTY_MIN) {
                servoState = RESTING;
            }
            break;
    }
}

static void wipersHi() {
    wipers.write(currentDuty);
    switch (servoState) {
        case RESTING:
            servoState = RISING;
            break;
        case RISING:
            // Move wipers out until they reach 67 degrees
            currentDuty += DUTY_INCREMENT_HI;
            if (currentDuty >= DUTY_67) {
                servoState = FALLING;
            }
            break;
        case FALLING:
            // Then retract them
            currentDuty -= DUTY_INCREMENT_HI;
            if (currentDuty <= DUTY_MIN) {
                servoState = RESTING;
            }
            break;
    }
}

static void wiperSelectorUpdate() {
    float reading = wiperSelect.read();

    // Force wipers to turn off if ignition is off
    if (!ignitionRead()) {
        if (wiperState != WIPERS_OFF) {
            wiperState = WIPERS_OFF;
            displayWriteMode("OFF ");
        }
        wiperState = WIPERS_OFF;
    }

    // Decimal values are used to separate areas of potentiometer into
    // HI, LO, INT, OFF sections. Padding used to prevent display flickering
    // State is only updated if previous state is different
    else if ( 0.75 + POT_SELECT_PADDING/2 < reading && wiperState != WIPERS_HI) {
        wiperState = WIPERS_HI;
        displayWriteMode("HIGH");
    }

    else if ( 0.50 + POT_SELECT_PADDING/2 < reading && reading < 0.75 - POT_SELECT_PADDING/2 && wiperState != WIPERS_LO) {
        wiperState = WIPERS_LO;
        displayWriteMode("LOW ");
            
    }

    else if ( 0.25 + POT_SELECT_PADDING/2 < reading && reading  < 0.50 - POT_SELECT_PADDING/2 && wiperState != WIPERS_INT) {
        wiperState = WIPERS_INT;
        displayWriteMode("INT ");
    }

    else if (reading < 0.25 - POT_SELECT_PADDING/2 && wiperState != WIPERS_OFF) {
        wiperState = WIPERS_OFF;
        displayWriteMode("OFF ");
    }
}


static int intSelectorUpdate() {
    // Similar to wiperSelector, but more modular
    float selectorReading = intSelect.read();
    for (int i=0; i<NUM_INT_SPEEDS-1 ; i++) {
        if (selectorReading < intSelectorThresholds[i]) {
            return intDelays[i];
        }
    }
    return intDelays[NUM_INT_SPEEDS-1];
}

static const char * intDelayToStr() {
    // Convert selected int delay to a string to display
    if (selectedIntDelay == intDelays[0]) {
        return "LONG  ";
    }
    else if (selectedIntDelay == intDelays[1]) {
        return "MEDIUM";
    }
    else {
        return "SHORT ";
    }
}