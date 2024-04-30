#pragma once

#include "common.h"
#include "types.h"

/******************************************************************/
static inline
hStatus actuator_set(act_type_t type, act_state_t state)
{
    hStatus status = hStatusOk;
    hLog(LOG_INFO, "ACTUATOR   [%10s] changed to %s",
         act_type_name[type],
         act_state_name[state]);

    return status;
}
/******************************************************************/
