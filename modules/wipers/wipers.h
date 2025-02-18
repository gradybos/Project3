//=====[#include guards - begin]===============================================

#ifndef _WIPERS_H_
#define _WIPERS_H_

//=====[Declarations of public datatypes]=========================

#include <cstdint>
typedef enum {
    WIPERS_HI,
    WIPERS_LO,
    WIPERS_INT,
    WIPERS_OFF
} wiperState_t;

//=====[Declarations (prototypes) of public functions]=========================

void wipersInit(int systemUpdateTime);
void wipersUpdate();
wiperState_t wipersRead();

int getSelectedIntDelay();

//=====[#include guards - end]=================================================

#endif //