#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "telegram.h"
#include "timer-helper.h"
#include "actuators.h"

/******************************************************************/
int main()
{
    bool is_running_ = true;
    hStatus status = hStatusOk;
    pthread_t thread_id;
    hConfig config;
    hSystemState sys_state = {
        .is_running = true,
        .is_light_on = false,
        .is_pump_on = false,
        .pH = 468,
        .EC = 1784
    };
    thread_state_t telegram_thread_state;
    struct sched_event event_light = {.actuator_type = ACT_TYPE_LIGTH};
    struct sched_event event_pump = {.actuator_type = ACT_TYPE_PUMP};
    hContainer container = {
        .config = &config,
        .state = &sys_state,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .telegram_thread_state = THREAD_STATE_RUNNING,
        .is_config_changed = false,
        .event_light = &event_light,
        .event_pump = &event_pump,
    };

    // -- SIGNAL handler -----------------------------------------------------
    void sighandler(int signum)
    {
        UNUSED(signum);
        is_running_ = false;
    }
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    // -- TIMER handler -----------------------------------------------------
    void timerhandler(int sig, siginfo_t *si, void *uc)
    {
        UNUSED(sig);
        UNUSED(uc);
        act_type_t actuator_type;
        act_state_t desired_state;
        hSchedEvent *sched_event = (hSchedEvent *) si->_sifields._rt.si_sigval.sival_ptr;
        if (!sched_event->is_done) {
            LOCK(sched_event->mutex);
            sched_event->is_done = true;
            sched_event->result = sched_event->result;
            actuator_type = sched_event->actuator_type;
            desired_state = sched_event->desired_state;
            UNLOCK(sched_event->mutex);

            hStatus res = actuator_set(actuator_type, desired_state);
            if(isOk(res)) {
                LOCK(container.mutex);
                if (actuator_type == ACT_TYPE_LIGTH) {
                    container.state->is_light_on = desired_state;
                }
                else if (actuator_type == ACT_TYPE_PUMP) {
                    container.state->is_pump_on = desired_state;
                }
                UNLOCK_CV(container.cond, container.mutex);
            }

            LOCK(sched_event->mutex);
            sched_event->result = res;
            UNLOCK(sched_event->mutex);

            timer_calc(sched_event, container.config, container.state);
        }
    }

    snprintf(container.config->config_dir, sizeof(container.config->config_dir), "%s/%s", getenv("HOME"), CONFIG_DIR);
    snprintf(container.config->config_file, sizeof(container.config->config_file), "%s/%s/%s", getenv("HOME"), CONFIG_DIR, CONFIG_FILENAME);

    do {
        status = read_config(container.config);
        if(!isOk(status)) {
            break;
        }
        status = timer_init(container.event_light, timerhandler);
        if(!isOk(status)) {
            break;
        }
        status = timer_init(container.event_pump, timerhandler);
        if(!isOk(status)) {
            break;
        }
        if (container.config->running) {
            status = timer_calc(container.event_light, container.config, container.state);
            if(!isOk(status)) {
                break;
            }
            status = timer_calc(container.event_pump, container.config, container.state);
            if(!isOk(status)) {
                break;
            }
            LOCK(container.mutex);
            container.state->is_running = true;
            UNLOCK_CV(container.cond, container.mutex);
        }
        else {
            hStatus res = actuator_set(container.event_light->actuator_type, ACT_ST_OFF);
            if(!isOk(res)) {
                hLog(LOG_ERR, "Could not switch off Light actuator: %u", res);
            }
            res = actuator_set(container.event_pump->actuator_type, ACT_ST_OFF);
            if(!isOk(res)) {
                hLog(LOG_ERR, "Could not switch off Pump: %u", res);
            }
            LOCK(container.mutex);
            container.state->is_running = false;
            UNLOCK_CV(container.cond, container.mutex);
        }

        pthread_create(&thread_id, NULL, telegram_thread, (void *)&container);

        while(is_running_) {
            bool is_config_changed = false;
            LOCK(container.mutex);
            is_config_changed = container.is_config_changed;
            telegram_thread_state = container.telegram_thread_state;
            if (is_config_changed) {
                if(!isOk(read_config(container.config))) {
                    save_config(container.config);
                }
            }
            UNLOCK_CV(container.cond, container.mutex);

            if (is_config_changed) {
                if (container.config->running) {
                    timer_calc(container.event_light, container.config, container.state);
                    timer_calc(container.event_pump, container.config, container.state);
                    if (!container.state->is_running) {
                        LOCK(container.mutex);
                        container.state->is_running = true;
                        UNLOCK_CV(container.cond, container.mutex);
                    }
                }
                else if (container.state->is_running) {
                    hStatus res = actuator_set(container.event_light->actuator_type, ACT_ST_OFF);
                    if(!isOk(res)) {
                        hLog(LOG_ERR, "Could not switch off Light actuator: %u", res);
                    }
                    res = timer_stop(container.event_light);
                    if(!isOk(res)) {
                        hLog(LOG_ERR, "Could not switch off Light timer: %u", res);
                    }

                    res = actuator_set(container.event_pump->actuator_type, ACT_ST_OFF);
                    if(!isOk(res)) {
                        hLog(LOG_ERR, "Could not switch off Pump: %u", res);
                    }
                    res = timer_stop(container.event_pump);
                    if(!isOk(res)) {
                        hLog(LOG_ERR, "Could not switch off Pump timer: %u", res);
                    }

                    LOCK(container.mutex);
                    container.state->is_running = false;
                    container.state->is_light_on = ACT_ST_OFF;
                    container.state->is_pump_on = ACT_ST_OFF;
                    UNLOCK_CV(container.cond, container.mutex);
                }
                LOCK(container.mutex);
                container.is_config_changed = false;
                UNLOCK_CV(container.cond, container.mutex);
            }

            if (telegram_thread_state == THREAD_STATE_FINISHED) {
                is_running_ = false;
                status = hStatusThreadError;
                break;
            }

            timer_get(container.event_light, &container.state->timer_light);
            timer_get(container.event_pump, &container.state->timer_pump);

            usleep(TIME_100MS_IN_US);
        }

        timer_teardown(container.event_light);
        timer_teardown(container.event_pump);
        LOCK(container.mutex);
        container.telegram_thread_state = THREAD_STATE_TERMINATION;
        UNLOCK_CV(container.cond, container.mutex);
        pthread_join(thread_id, NULL);
    } while(0);

    hLog(LOG_INFO, "Hydroponic system is finished, status: %u, reason: %s",
         status,
         (!is_running_) ? "recived termination signal" : strerror(errno));

    return status;
}
