#pragma once

#include "types.h"

#define SCHED_LIGHT_ON_DEFAULT          5
#define SCHED_LIGHT_OFF_DEFAULT         20
#define SCHED_PUMP_ON_DEFAULT           15
#define SCHED_PUMP_OFF_DEFAULT          25
#define SCHED_PUMP_NIGHT_DEFAULT        2
#define CONFIG_RUNNING_DEFAULT          false
#define CONFIG_PH_DEFAULT               630
#define CONFIG_EC_DEFAULT               1300
#define CONFIG_DIR                      ".el_garden"
#define CONFIG_FILENAME                 "config.json"

#define PORT                            (2300)
#define POLL_TIMEOUT_MS                 (500)

#define TIME_1S_IN_NS                   (1000000000)
#define TIME_20MS_IN_NS                 (20000000)
#define TIME_100MS_IN_US                (100000)
#define TIME_NS_MAX                     (1000000000)

#define COMMAND_SET_CFG                 "set_config"
#define COMMAND_GET_CFG                 "get_config"
#define COMMAND_UPDATE_DATETIME         "update_datetime"
#define COMMAND_GET_SYS_STATE           "get_sys_state"
#define COMMAND_SYSTEM_STOP             "system_stop"
#define COMMAND_SYSTEM_START            "system_start"

/******************************************************************/
