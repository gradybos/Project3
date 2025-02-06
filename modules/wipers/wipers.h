//=====[#include guards - begin]===============================================

#ifndef _WIPERS_H_
#define _WIPERS_H_

//=====[Declarations of public datatypes]=========================

typedef enum {
    HI,
    LO,
    INT,
    OFF
} wiperState_t;

//=====[Declarations (prototypes) of public functions]=========================

void wipersInit();
void wipersUpdate();
wiperState_t wipersRead();

//=====[#include guards - end]=================================================

#endif //