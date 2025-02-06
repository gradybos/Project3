#include "mbed.h"
#include "car_system.h"

// main() runs in its own thread in the OS
int main()
{
    carSystemInit();
    while (true) {
        carSystemUpdate();
    }
}

