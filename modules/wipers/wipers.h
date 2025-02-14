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

void wipersInit();
void wipersUpdate(int systemUpdateTime);
wiperState_t wipersRead();

//=====[#include guards - end]=================================================

#endif //