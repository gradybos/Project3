//=====[Libraries]=============================================================

#include "mbed.h"
#include "alarm.h"
#include "arm_book_lib.h"

//=====[Declaration and initialization of private global variables]============

DigitalInOut alarm(PE_10);
static bool alarmEnabled = false;

//=====[Implementations of public functions]===================================

void alarmInit() {
    alarm.mode(OpenDrain);
    alarm.input();
}

void alarmEnable() {
    alarm.output();
    alarmEnabled = true;
}

void alarmDisable() {
    alarm.input();
    alarmEnabled = false;
}

bool alarmRead() {
    return alarmEnabled;
}