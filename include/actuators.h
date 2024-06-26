#pragma once

#include "common.h"
#include "types.h"

#if defined BUILD_PLATFORM_ARM
#include <pigpio.h>
#else
#define gpioInitialise() ({ int result = 0; result; })
#endif
/******************************************************************/
static inline
hStatus actuator_init()
{
    hStatus status = hStatusOk;
    if (gpioInitialise() < 0)
    {
        hLog(LOG_ERR, "Failed GPIO initialization");
       // pigpio initialisation failed.
    }
    else
    {
        hLog(LOG_INFO, "GPIO initialization successful");
       // pigpio initialised okay.
    }

    return status;
}
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
