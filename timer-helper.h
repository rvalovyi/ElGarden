#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "types.h"
#include "common.h"

/******************************************************************/
uint32_t get_night_duration_min (uint32_t sched_light_on, uint32_t sched_light_off)
{
    return (sched_light_off > sched_light_on)
            ? (sched_light_off - sched_light_on) * 60
            : (24 - sched_light_on + sched_light_off) * 60;
}
/******************************************************************/
static inline
hStatus timer_get(hSchedEvent *sched_event, uint32_t *tv_sec)
{
    hStatus status = hStatusOk;

    int res = 0;
    timer_t timer_id = 0;
    struct itimerspec its = {0};

    do {
        LOCK(sched_event->mutex);
        timer_id = sched_event->timer_id;
        UNLOCK(sched_event->mutex);

        /* start timer */
        res = timer_gettime(timer_id, &its);
        if ( res != 0){
            hLog(LOG_ERR, "Timer get failed: %s", strerror(errno));
            status = hStatusTimerSetTimeError;
            break;
        }

        *tv_sec = its.it_value.tv_sec;

    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus timer_stop(hSchedEvent *sched_event)
{
    hStatus status = hStatusOk;
    timer_t timer_id = 0;

    do {
        LOCK(sched_event->mutex);
        timer_id = sched_event->timer_id;
        sched_event->is_done = true;
        UNLOCK(sched_event->mutex);

        struct itimerspec its = {   .it_value.tv_sec  = 0,
                                    .it_value.tv_nsec = 0,
                                    .it_interval.tv_sec  = 0,
                                    .it_interval.tv_nsec = 0
                                };
        int res = timer_settime(timer_id, 0, &its, NULL);
        if ( res != 0){
            hLog(LOG_ERR, "Timer stop failed: %s", strerror(errno));
            status = hStatusTimerSetTimeError;
            break;
        }
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus timer_start(hSchedEvent *sched_event, uint32_t tv_sec, act_state_t desired_state)
{
    hStatus status = hStatusOk;

    int res = 0;
    timer_t timer_id = 0;

    /* specify start delay and interval */
    struct itimerspec its = {   .it_value.tv_sec  = tv_sec,
                                .it_value.tv_nsec = 0,
                                .it_interval.tv_sec  = 0,
                                .it_interval.tv_nsec = 0
                            };

    do {
        LOCK(sched_event->mutex);
        timer_id = sched_event->timer_id;
        UNLOCK(sched_event->mutex);

        /* start timer */
        res = timer_settime(timer_id, 0, &its, NULL);
        if ( res != 0){
            hLog(LOG_ERR, "Timer start failed: %s", strerror(errno));
            status = hStatusTimerSetTimeError;
            break;
        }

        LOCK(sched_event->mutex);
        sched_event->timer_id = timer_id;
        sched_event->desired_state = desired_state;
        UNLOCK(sched_event->mutex);

    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus timer_init(hSchedEvent *sched_event, void* handler)
{
    hStatus status = hStatusOk;

    int res = 0;
    timer_t timer_id = 0;

    struct sigevent sev = { 0 };

    /* specifies the action when receiving a signal */
    struct sigaction sa = { 0 };

    sev.sigev_notify = SIGEV_SIGNAL; // Linux-specific
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = sched_event;

    do {
        /* create timer */
        res = timer_create(CLOCK_REALTIME, &sev, &timer_id);
        if ( res != 0){
            hLog(LOG_ERR, "Timer creation failed: %s", strerror(errno));
            status = hStatusTimerCreateError;
            break;
        }

        /* specifz signal and handler ligth */
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler;

        /* Initialize signal */
        sigemptyset(&sa.sa_mask);

        /* Register signal handler ligth */
        if (sigaction(SIGRTMIN, &sa, NULL) == -1){
            hLog(LOG_ERR, "Timer sigaction failed: %s", strerror(errno));
            status = hStatusTimerSigactionError;
            break;
        }

        LOCK(sched_event->mutex);
        sched_event->timer_id = timer_id;
        sched_event->is_done = true;
        UNLOCK(sched_event->mutex);

        /* stop timer */
        status = timer_stop(sched_event);

    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus timer_teardown(hSchedEvent *sched_event)
{
    hStatus status = hStatusOk;
    timer_t timer_id = 0;

    do {
        LOCK(sched_event->mutex);
        timer_id = sched_event->timer_id;
        sched_event->is_done = true;
        UNLOCK(sched_event->mutex);

        int res = timer_delete(timer_id);
        if ( res != 0){
            hLog(LOG_ERR, "Timer delete failed: %s", strerror(errno));
            status = hStatusTimerDeleteError;
            break;
        }
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus timer_calc(hSchedEvent *sched_event, hConfig *config, hSystemState *state)
{
    hStatus status = hStatusOk;
    act_type_t actuator_type;
    bool is_light_on = false;
    bool is_pump_on = false;
    bool is_night_now = false;
    uint32_t tv_sec = 0;
    hDateTime dtime;
    uint32_t sched_light_on;
    uint32_t sched_light_off;
    uint32_t hours_passed_since_start = 0;
    uint32_t passed_since_start_sec = 0;
    uint32_t total_duration_hour = 0;
    uint32_t total_duration_sec = 0;
    uint32_t sched_pump_on_sec = 0;
    uint32_t sched_pump_off_sec = 0;
    uint32_t sched_pump_night = 0;
    act_state_t desired_state;
    char time[BUFSIZ];

    LOCK(sched_event->mutex);
    do {
        if (!sched_event || !config || !state) {
            hLog(LOG_ERR, "%s Argument unexpected", __func__);
            status = hStatusInternalError;
            break;
        }
        status = get_datettime(&dtime, 0, 0, DATETIME_TYPEOUT_BIN);
        if (!isOk(status)) {
            break;
        }

        is_light_on = state->is_light_on;
        is_pump_on = state->is_pump_on;
        actuator_type = sched_event->actuator_type;
        sched_event->is_done = false;
        sched_light_on = config->sched_light_on;
        sched_light_off = config->sched_light_off;
        sched_pump_on_sec = config->sched_pump_on * 60;
        sched_pump_off_sec = config->sched_pump_off * 60;
        sched_pump_night = config->sched_pump_night;

        if (sched_light_off > sched_light_on) {
            if (dtime.tm_hour >= sched_light_on && dtime.tm_hour < sched_light_off) {
                hours_passed_since_start = dtime.tm_hour - sched_light_on;
                is_night_now = false;
                total_duration_hour = sched_light_off - sched_light_on;
            }
            if (dtime.tm_hour < sched_light_on ||
                dtime.tm_hour >= sched_light_off) {
                if (dtime.tm_hour >= sched_light_off) {
                    hours_passed_since_start = dtime.tm_hour - sched_light_off;
                }
                else {
                    hours_passed_since_start = 24 - sched_light_off + dtime.tm_hour;
                }
                is_night_now = true;
                total_duration_hour = 24 - (sched_light_off - sched_light_on);
            }
        }
        else {
            if (dtime.tm_hour >= sched_light_off && dtime.tm_hour < sched_light_on) {
                hours_passed_since_start = dtime.tm_hour - sched_light_off;
                is_night_now = true;
                total_duration_hour = sched_light_on - sched_light_off;
            }
            else {
                if (dtime.tm_hour >= sched_light_on) {
                    hours_passed_since_start = dtime.tm_hour - sched_light_on;
                }
                else {
                    hours_passed_since_start = 24 - sched_light_on + dtime.tm_hour;
                }
                is_night_now = false;
                total_duration_hour = 24 - (sched_light_on - sched_light_off);
            }
        }

//        hLog(LOG_DEBUG, "TIMER CALC [%10s] %d.%d.%d %02d:%02d:%02d",
//             act_type_name[actuator_type],
//             dtime.tm_mday,
//             dtime.tm_mon,
//             dtime.tm_year,
//             dtime.tm_hour,
//             dtime.tm_min,
//             dtime.tm_sec);
        hLog(LOG_INFO, "TIMER CALC [%10s] It is %s; Light: %s; Pump: %s",
             act_type_name[actuator_type],
             is_night_now ? "Night" : "Day",
             is_light_on ? act_state_name[ACT_ST_ON] : act_state_name[ACT_ST_OFF],
             is_pump_on ? act_state_name[ACT_ST_ON] : act_state_name[ACT_ST_OFF] );

        passed_since_start_sec = hours_passed_since_start * 3600 + dtime.tm_min * 60 + dtime.tm_sec;
        total_duration_sec = total_duration_hour * 3600;

//        hLog(LOG_DEBUG, "It's %s", is_night_now ? "night" : "day");
//        hLog(LOG_DEBUG, "total_duration_hour %u total_duration_sec %u", total_duration_hour, total_duration_sec);
//        hLog(LOG_DEBUG, "passed_since_start_sec %u", passed_since_start_sec);

        if (actuator_type == ACT_TYPE_LIGTH) {
            tv_sec = total_duration_sec - passed_since_start_sec;
            if (is_night_now) {
                if (is_light_on) {
                    desired_state = ACT_ST_OFF;
                    tv_sec = 1;
                }
                else {
                    desired_state = ACT_ST_ON;
                }
            }
            else {
                if (!is_light_on) {
                    desired_state = ACT_ST_ON;
                    tv_sec = 1;
                }
                else {
                    desired_state = ACT_ST_OFF;
                }
            }
        }
        else if (actuator_type == ACT_TYPE_PUMP) {
            uint32_t pump_period_sec = sched_pump_on_sec + sched_pump_off_sec;
            uint32_t left_to_end_sec = total_duration_sec - passed_since_start_sec;
//            hLog(LOG_DEBUG, "pump_period_sec %u", pump_period_sec);
            if (is_night_now) {
                uint32_t total_pump_period_sec = pump_period_sec * sched_pump_night;
                uint32_t idle_period_sec = (total_duration_sec - total_pump_period_sec) / (sched_pump_night + 1);
//                hLog(LOG_DEBUG, "total_pump_period_sec %u", total_pump_period_sec);
//                hLog(LOG_DEBUG, "| IDLE %u | ON %u | OFF %u |", idle_period_sec, sched_pump_on_sec, sched_pump_off_sec);
                for (int i = 0; i <= sched_pump_night; i++) {
                    uint32_t duration_prev_cycles_sec = (idle_period_sec + pump_period_sec) * i;
//                    hLog(LOG_DEBUG, "[%u] duration_prev_cycles_sec %u", i, duration_prev_cycles_sec);
                    if (passed_since_start_sec < (idle_period_sec + pump_period_sec) * (i + 1)) {
                        uint32_t passed_since_cycle_start_sec = passed_since_start_sec - duration_prev_cycles_sec;
//                        hLog(LOG_DEBUG, "   Passed through a cycle: %u", passed_since_cycle_start_sec);
                        if (passed_since_cycle_start_sec < idle_period_sec) {
                            if (is_pump_on) {
                                desired_state = ACT_ST_OFF;
                                tv_sec = 1;
                            }
                            else {
                                desired_state = ACT_ST_ON;
                                tv_sec = idle_period_sec - passed_since_cycle_start_sec;
                            }
//                            hLog(LOG_DEBUG, "   Choised 1 tv_sec %u", tv_sec);
                        }
                        else if (passed_since_cycle_start_sec >= idle_period_sec &&
                                 passed_since_cycle_start_sec < idle_period_sec + sched_pump_on_sec) {
                            if (!is_pump_on) {
                                desired_state = ACT_ST_ON;
                                tv_sec = 1;
                            }
                            else {
                                desired_state = ACT_ST_OFF;
                                tv_sec = (idle_period_sec + sched_pump_on_sec) - passed_since_cycle_start_sec;
                            }
//                            hLog(LOG_DEBUG, "   Choised 2 tv_sec %u", tv_sec);
                        }
                        else if (passed_since_cycle_start_sec >= idle_period_sec + sched_pump_on_sec &&
                                 passed_since_cycle_start_sec < idle_period_sec + sched_pump_off_sec) {
                            desired_state = ACT_ST_OFF;
                            if (is_pump_on) {
                                tv_sec = 1;
                            }
                            else {
                                tv_sec = (idle_period_sec + pump_period_sec) - passed_since_cycle_start_sec;
                            }
//                            hLog(LOG_DEBUG, "   Choised 3 tv_sec %u", tv_sec);
                        }
                        if (tv_sec >= left_to_end_sec) {
                            desired_state = ACT_ST_ON;
                            tv_sec = left_to_end_sec;
                        }

                        break;
                    }
                }
            }
            else { // is day now
//                hLog(LOG_DEBUG, "| ON %u | OFF %u |", sched_pump_on_sec, sched_pump_off_sec);
                for (int i = 0; i <= total_duration_sec / pump_period_sec; i++) {
                    uint32_t duration_prev_cycles_sec = pump_period_sec * i;
//                    hLog(LOG_DEBUG, "[%u] duration_prev_cycles_sec %u", i, duration_prev_cycles_sec);
                    if (passed_since_start_sec < pump_period_sec * (i + 1)) {
                        uint32_t passed_since_cycle_start_sec = passed_since_start_sec - duration_prev_cycles_sec;
//                        hLog(LOG_DEBUG, "   Passed through a cycle: %u", passed_since_cycle_start_sec);
                        if (passed_since_cycle_start_sec < sched_pump_on_sec) {
                            if (!is_pump_on) {
                                desired_state = ACT_ST_ON;
                                tv_sec = 1;
                            }
                            else {
                                desired_state = ACT_ST_OFF;
                                tv_sec = sched_pump_on_sec - passed_since_cycle_start_sec;
                            }
//                            hLog(LOG_DEBUG, "   Choised 1 tv_sec %u", tv_sec);
                        }
                        else if (passed_since_cycle_start_sec >= sched_pump_on_sec &&
                                 passed_since_cycle_start_sec < sched_pump_on_sec + sched_pump_off_sec) {
                            if (is_pump_on) {
                                desired_state = ACT_ST_OFF;
                                tv_sec = 1;
                            }
                            else {
                                desired_state = ACT_ST_ON;
                                tv_sec = (sched_pump_on_sec + sched_pump_off_sec) - passed_since_cycle_start_sec;
                            }
//                            hLog(LOG_DEBUG, "   Choised 2 tv_sec %u", tv_sec);
                        }
                        if (tv_sec >= left_to_end_sec) {
                            desired_state = ACT_ST_OFF;
                            tv_sec = left_to_end_sec;
                        }
                        break;
                    }
                }
            }
        }
        else {
            hLog(LOG_ERR, "Unknown actuator type %u", actuator_type);
            break;
        }

        if (isOk(get_datettime(time, BUFSIZ, tv_sec, DATETIME_TYPEOUT_TIME))) {
            hLog(LOG_INFO, "TIMER CALC [%10s] Scheduled: sec %u (at %s) desired_state %s", act_type_name[actuator_type], tv_sec, time, act_state_name[desired_state]);
        }

    } while(0);
    UNLOCK(sched_event->mutex);

    if (isOk(status)) {
        status = timer_start(sched_event, tv_sec, desired_state);
    }
    hLog(LOG_INFO, "===========================");

    if(!isOk(status)) {
        hLog(LOG_ERR, "timer_calc() timer for %s failed, status %u error '%s'",
             act_type_name[actuator_type],
             status,
             strerror(errno));
    }

    return status;
}
/******************************************************************/
