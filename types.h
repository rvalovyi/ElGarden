#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

/******************************************************************/
typedef enum _act_type_t {
    ACT_TYPE_LIGTH,
    ACT_TYPE_PUMP,

    ACT_TYPE_MAX,
} act_type_t;
/******************************************************************/
typedef enum _act_state_t {
    ACT_ST_OFF,
    ACT_ST_ON,

    ACT_ST_MAX
} act_state_t;
/******************************************************************/
typedef enum _thread_state_t {
    THREAD_STATE_RUNNING,
    THREAD_STATE_TERMINATION,
    THREAD_STATE_FINISHED
} thread_state_t;
/******************************************************************/
const char *act_type_name[] = {
    [ACT_TYPE_LIGTH] = "Light",
    [ACT_TYPE_PUMP] = "Pump"
};
/******************************************************************/
const char *act_state_name[] = {
    [ACT_ST_ON] = "ON",
    [ACT_ST_OFF] = "OFF"
};
/******************************************************************/
static_assert((sizeof(act_type_name)/sizeof(*(act_type_name))) <= ACT_TYPE_MAX);
static_assert((sizeof(act_state_name)/sizeof(*(act_state_name))) <= ACT_ST_MAX);
/******************************************************************/
struct config {
    char config_dir[FILENAME_MAX];
    char config_file[FILENAME_MAX];
    bool running;               // System is running or stopped
    uint32_t sched_light_on;    // light switch-on time [0..23] (hours)
    uint32_t sched_light_off;   // light switch-off time [0..23] (hours)
    uint32_t sched_pump_on;     // pump on time [1..60] (min)
    uint32_t sched_pump_off;    // pump off time [1..60] (min)
    uint32_t sched_pump_night;  // how many times the pump should be switched on per night [0..10]
    uint32_t pH;                // desired pH value
    uint32_t EC;                // desired EC value
};
/******************************************************************/
struct state {
    uint32_t pH;
    uint32_t EC;
    uint32_t timer_light;
    uint32_t timer_pump;
    bool is_running;
    bool is_light_on;
    bool is_pump_on;
};
/******************************************************************/
struct sched_event {
    pthread_mutex_t mutex;
    act_type_t actuator_type;
    act_state_t desired_state;
    timer_t timer_id;
    int result;
    bool is_done;
};
/******************************************************************/
struct container {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct config *config;
    struct state *state;
    struct sched_event *event_light;
    struct sched_event *event_pump;
    bool is_config_changed;
    thread_state_t telegram_thread_state;
};
/******************************************************************/
struct date_time {
    uint32_t tm_sec;		/* Seconds.	[0-60] */
    uint32_t tm_min;		/* Minutes.	[0-59] */
    uint32_t tm_hour;		/* Hours.	[0-23] */
    uint32_t tm_mday;		/* Day.		[1-31] */
    uint32_t tm_mon;		/* Month.	[1-12] */
    uint32_t tm_year;		/* Year.  */
};
/******************************************************************/
typedef struct config hConfig;
typedef struct state hSystemState;
typedef struct sched_event hSchedEvent;
typedef struct container hContainer;
typedef struct date_time hDateTime;
/******************************************************************/
