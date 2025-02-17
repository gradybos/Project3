//=====[Libraries]=============================================================

#include "arm_book_lib.h"
#include "mbed.h"
#include "car_system.h"
#include "ignition.h"
#include "wipers.h"
#include "display.h"

//=====[Implementations of public functions]===================================

#define SYSTEM_UPDATE_TIME_MS 10

void carSystemInit()
{
    ignitionInit();
    displayInit();
    wipersInit();
}

void carSystemUpdate()
{
    ignitionUpdate();
    wipersUpdate(SYSTEM_DELAY_MS);
}