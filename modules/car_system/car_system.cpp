//=====[Libraries]=============================================================

#include "arm_book_lib.h"
#include "mbed.h"
#include "car_system.h"
#include "ignition.h"
#include "headlights.h"

//=====[Implementations of public functions]===================================

void carSystemInit()
{
    ignitionInit();
    headlightsInit();
}

void carSystemUpdate()
{
    ignitionUpdate();
    headlightsUpdate();
}