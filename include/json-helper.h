#pragma once

#include <json-c/json.h>

#include "defines.h"
#include "common.h"
#include "timer-helper.h"

/******************************************************************/
static inline
hStatus system_stop(void)
{
    hStatus status = hStatusOk;

    return status;
}
/******************************************************************/
static inline
hStatus system_start(void)
{
    hStatus status = hStatusOk;

    return status;
}
/******************************************************************/
static inline
hStatus update_datettime(void)
{
    hStatus status = hStatusOk;

    return status;
}
/******************************************************************/
static inline
hStatus save_config(hConfig *cfg)
{
    hStatus status = hStatusOk;
    const char* json_dir = (const char*)cfg->config_dir;
    const char* json_file = (const char*)cfg->config_file;
    do {

        json_object *root = json_object_new_object();
        if (!root) {
            status = hStatusJsonObjCreateError;
            break;
        }
        json_object_object_add(root, "running", json_object_new_int(cfg->running));
        json_object_object_add(root, "light_on", json_object_new_int(cfg->sched_light_on));
        json_object_object_add(root, "light_off", json_object_new_int(cfg->sched_light_off));
        json_object_object_add(root, "pump_on", json_object_new_int(cfg->sched_pump_on));
        json_object_object_add(root, "pump_off", json_object_new_int(cfg->sched_pump_off));
        json_object_object_add(root, "pump_night", json_object_new_int(cfg->sched_pump_night));
        json_object_object_add(root, "pH", json_object_new_int(cfg->pH));
        json_object_object_add(root, "EC", json_object_new_int(cfg->EC));

        // save json
        status = isDirExists(json_dir);
        if (isOk(status)) {
            if (json_object_to_file_ext(json_file, root, JSON_C_TO_STRING_PRETTY)) {
                hLog(LOG_ERR, "It's impossible to save config to file: %s", json_file);
                status = hStatusSaveFileError;
                break;
            }
            else {
                hLog(LOG_DEBUG, "%s saved.", json_file);
            }
        }

        // cleanup and exit
        json_object_put(root);
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus read_config(hConfig *cfg)
{
    hStatus status = hStatusOk;
    const char* json_file = (const char*)cfg->config_file;

    cfg->running = CONFIG_RUNNING_DEFAULT;
    cfg->sched_light_on = SCHED_LIGHT_ON_DEFAULT;
    cfg->sched_light_off = SCHED_LIGHT_OFF_DEFAULT;
    cfg->sched_pump_on = SCHED_PUMP_ON_DEFAULT;
    cfg->sched_pump_off = SCHED_PUMP_OFF_DEFAULT;
    cfg->sched_pump_night = SCHED_PUMP_NIGHT_DEFAULT;
    cfg->pH = CONFIG_PH_DEFAULT;
    cfg->EC = CONFIG_EC_DEFAULT;

    json_object *root = json_object_from_file(json_file);
    if (!root) {
        root = json_object_new_object();
        if (!root) {
            hLog(LOG_ERR, "Failed to create json object [%s]", strerror(errno));
            status = hStatusJsonObjCreateError;
        }
    }

    if (isOk(status)) {
        json_object *running = json_object_object_get(root, "running");
        if (running) {
            cfg->running = json_object_get_boolean(running);
        }
        json_object *light_on = json_object_object_get(root, "light_on");
        if (light_on) {
            cfg->sched_light_on = json_object_get_int(light_on);
        }
        json_object *light_off = json_object_object_get(root, "light_off");
        if (light_off) {
            cfg->sched_light_off = json_object_get_int(light_off);
        }
        json_object *pump_on = json_object_object_get(root, "pump_on");
        if (pump_on) {
            cfg->sched_pump_on = json_object_get_int(pump_on);
        }
        json_object *pump_off = json_object_object_get(root, "pump_off");
        if (pump_off) {
            cfg->sched_pump_off = json_object_get_int(pump_off);
        }
        json_object *pump_night = json_object_object_get(root, "pump_night");
        if (pump_night) {
            cfg->sched_pump_night = json_object_get_int(pump_night);
        }
        json_object *pH = json_object_object_get(root, "pH");
        if (pH) {
            cfg->pH = json_object_get_int(pH);
        }
        json_object *EC = json_object_object_get(root, "EC");
        if (EC) {
            cfg->EC = json_object_get_int(EC);
        }
    }

    return status;
}
/******************************************************************/
static inline
hStatus get_sys_state(json_object *param_obj, json_object *janswer, hSystemState *sys_state)
{
    hStatus status = hStatusParseError;
    char date[BUFSIZ];
    char time[BUFSIZ];
    char timer_chg_light[BUFSIZ];
    char timer_chg_pump[BUFSIZ];

    do {
        char *str_param = (char *) json_object_get_string(param_obj);
        if (!str_param) {
            break;
        }
        bzero(date, BUFSIZ);
        bzero(time, BUFSIZ);
        status = get_datettime(date, BUFSIZ, 0, DATETIME_TYPEOUT_DATE);
        if (!isOk(status)) {
            break;
        }
        status = get_datettime(time, BUFSIZ, 0, DATETIME_TYPEOUT_TIME);
        if (!isOk(status)) {
            break;
        }
        status = get_datettime(timer_chg_light, BUFSIZ, sys_state->timer_light, DATETIME_TYPEOUT_TIME);
        if (!isOk(status)) {
            break;
        }
        status = get_datettime(timer_chg_pump, BUFSIZ, sys_state->timer_pump, DATETIME_TYPEOUT_TIME);
        if (!isOk(status)) {
            break;
        }

        if (!strncmp(str_param, "all", strlen(str_param))) {
            json_object_object_add(janswer, "is_running", json_object_new_boolean(sys_state->is_running));
            json_object_object_add(janswer, "is_light_on", json_object_new_boolean(sys_state->is_light_on));
            json_object_object_add(janswer, "is_pump_on", json_object_new_boolean(sys_state->is_pump_on));
            json_object_object_add(janswer, "pH", json_object_new_int(sys_state->pH));
            json_object_object_add(janswer, "EC", json_object_new_int(sys_state->EC));
            if (strlen(date) && strlen(time)) {
                json_object_object_add(janswer, "date", json_object_new_string(date));
                json_object_object_add(janswer, "time", json_object_new_string(time));
            }
            else {
                json_object_object_add(janswer, "date", json_object_new_string("-"));
                json_object_object_add(janswer, "time", json_object_new_string("-"));
            }
            if (sys_state->is_running) {
                json_object_object_add(janswer, "timer_chg_light", json_object_new_string(timer_chg_light));
                json_object_object_add(janswer, "timer_chg_pump", json_object_new_string(timer_chg_pump));
            }
            else {
                json_object_object_add(janswer, "timer_chg_light", json_object_new_string("-"));
                json_object_object_add(janswer, "timer_chg_pump", json_object_new_string("-"));
           }
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "is_light_on", strlen(str_param))) {
            json_object_object_add(janswer, "is_light_on", json_object_new_boolean(sys_state->is_light_on));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "is_pump_on", strlen(str_param))) {
            json_object_object_add(janswer, "is_pump_on", json_object_new_boolean(sys_state->is_pump_on));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "pH", strlen(str_param))) {
            json_object_object_add(janswer, "pH", json_object_new_int(sys_state->pH));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "EC", strlen(str_param))) {
            json_object_object_add(janswer, "EC", json_object_new_int(sys_state->EC));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "is_running", strlen(str_param))) {
            json_object_object_add(janswer, "is_running", json_object_new_boolean(sys_state->is_running));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "timer_chg_light", strlen(str_param))) {
            if (sys_state->is_running) {
                json_object_object_add(janswer, "timer_chg_light", json_object_new_string(timer_chg_light));
            }
            else {
                json_object_object_add(janswer, "timer_chg_light", json_object_new_string("-"));
            }
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "timer_chg_pump", strlen(str_param))) {
            if (sys_state->is_running) {
                json_object_object_add(janswer, "timer_chg_pump", json_object_new_string(timer_chg_pump));
            }
            else {
                json_object_object_add(janswer, "timer_chg_pump", json_object_new_string("-"));
            }
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "date", strlen(str_param))) {
            if (strlen(date) && strlen(time)) {
                json_object_object_add(janswer, "date", json_object_new_string(date));
            }
            else {
                json_object_object_add(janswer, "date", json_object_new_string("-"));
            }
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "time", strlen(str_param))) {
            if (strlen(date) && strlen(time)) {
                json_object_object_add(janswer, "time", json_object_new_string(time));
            }
            else {
                json_object_object_add(janswer, "time", json_object_new_string("-"));
            }
            status = hStatusOk;
        }
        else {
            status = hStatusParamUnknown;
        }
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus set_config(json_object *param_obj, hContainer *container)
{
    hStatus status = hStatusParseError;
    hConfig cfg_new;

    do {
        memcpy(&cfg_new, container->config, sizeof(hConfig));

        json_object *running = json_object_object_get(param_obj, "running");
        if (running) {
            cfg_new.running = json_object_get_boolean(running);
        }

        json_object *light_on = json_object_object_get(param_obj, "light_on");
        if (light_on) {
            cfg_new.sched_light_on = json_object_get_int(light_on);
            if (cfg_new.sched_light_on > 23) {
                hLog(LOG_ERR, "Wrong 'light_on' param [0..23]: %d", cfg_new.sched_light_on);
                status = hStatusCfgLightError;
                break;
            }
        }

        json_object *light_off = json_object_object_get(param_obj, "light_off");
        if (light_off) {
            cfg_new.sched_light_off = json_object_get_int(light_off);
            if (cfg_new.sched_light_on > 23) {
                hLog(LOG_ERR, "Wrong 'light_off' param [0..23]: %d", cfg_new.sched_light_off);
                status = hStatusCfgLightError;
                break;
            }
        }

        if (cfg_new.sched_light_on == cfg_new.sched_light_off) {
            hLog(LOG_ERR, "'light_off' and 'light_on' can't be equal");
            status = hStatusCfgLightError;
            break;
        }

        json_object *pump_on = json_object_object_get(param_obj, "pump_on");
        if (pump_on) {
            cfg_new.sched_pump_on = json_object_get_int(pump_on);
            if (cfg_new.sched_pump_on == 0 || cfg_new.sched_pump_on > 60) {
                hLog(LOG_ERR, "Wrong 'pump_on' param [1..60]: %d", cfg_new.sched_pump_off);
                status = hStatusCfgPumpError;
                break;
            }
        }

        json_object *pump_off = json_object_object_get(param_obj, "pump_off");
        if (pump_off) {
            cfg_new.sched_pump_off = json_object_get_int(pump_off);
            if (cfg_new.sched_pump_off == 0 || cfg_new.sched_pump_off > 60) {
                hLog(LOG_ERR, "Wrong 'pump_off' param [1..60]: %d", cfg_new.sched_pump_off);
                status = hStatusCfgPumpError;
                break;
            }
        }

        json_object *pump_night = json_object_object_get(param_obj, "pump_night");
        if (pump_night) {
            cfg_new.sched_pump_night = json_object_get_int(pump_night);
            uint32_t pump_duration_min = cfg_new.sched_pump_night * (cfg_new.sched_pump_on + cfg_new.sched_pump_off);
            uint32_t night_duration_min = get_night_duration_min(cfg_new.sched_light_on, cfg_new.sched_light_off);
    //                printf("pump_duration_min: %u\n", pump_duration_min);
    //                printf("night_duration_min: %u\n", night_duration_min);
            if (pump_duration_min > night_duration_min) {
                hLog(LOG_ERR, "It's impossible to turn the pump on so many times a night");
                status = hStatusCfgPumpError;
                break;
            }
        }

        json_object *pH = json_object_object_get(param_obj, "pH");
        if (pH) {
            cfg_new.pH = json_object_get_int(pH);
            if (cfg_new.pH == 0 || cfg_new.pH > 3000) {
                hLog(LOG_ERR, "Wrong 'pH' param [1..3000]: %d", cfg_new.pH);
                status = hStatusCfgPHError;
                break;
            }
        }

        json_object *EC = json_object_object_get(param_obj, "EC");
        if (EC) {
            cfg_new.EC = json_object_get_int(EC);
            if (cfg_new.EC == 0 || cfg_new.pH > 7000) {
                hLog(LOG_ERR, "Wrong 'pH' param [1..7000]: %d", cfg_new.EC);
                status = hStatusCfgECError;
                break;
            }
        }

        status = save_config(&cfg_new);
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus get_config(json_object *param_obj, json_object *janswer, hConfig *cfg)
{
    hStatus status = hStatusParseError;

    do {
        char *str_param = (char *) json_object_get_string(param_obj);
        if (!str_param) {
            break;
        }

        if (!strncmp(str_param, "all", strlen(str_param))) {
            json_object_object_add(janswer, "running", json_object_new_int(cfg->running));
            json_object_object_add(janswer, "light_on", json_object_new_int(cfg->sched_light_on));
            json_object_object_add(janswer, "light_off", json_object_new_int(cfg->sched_light_off));
            json_object_object_add(janswer, "pump_on", json_object_new_int(cfg->sched_pump_on));
            json_object_object_add(janswer, "pump_off", json_object_new_int(cfg->sched_pump_off));
            json_object_object_add(janswer, "pump_night", json_object_new_int(cfg->sched_pump_night));
            json_object_object_add(janswer, "pH", json_object_new_int(cfg->pH));
            json_object_object_add(janswer, "EC", json_object_new_int(cfg->EC));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "running", strlen(str_param))) {
            json_object_object_add(janswer, "running", json_object_new_boolean(cfg->running));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "light_on", strlen(str_param))) {
            json_object_object_add(janswer, "light_on", json_object_new_int(cfg->sched_light_on));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "light_off", strlen(str_param))) {
            json_object_object_add(janswer, "light_off", json_object_new_int(cfg->sched_light_off));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "pump_on", strlen(str_param))) {
            json_object_object_add(janswer, "pump_on", json_object_new_int(cfg->sched_pump_on));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "pump_off", strlen(str_param))) {
            json_object_object_add(janswer, "pump_off", json_object_new_int(cfg->sched_pump_off));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "pump_night", strlen(str_param))) {
            json_object_object_add(janswer, "pump_night", json_object_new_int(cfg->sched_pump_night));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "pH", strlen(str_param))) {
            json_object_object_add(janswer, "pH", json_object_new_int(cfg->pH));
            status = hStatusOk;
        }
        else if (!strncmp(str_param, "EC", strlen(str_param))) {
            json_object_object_add(janswer, "EC", json_object_new_int(cfg->EC));
            status = hStatusOk;
        }
        else {
            status = hStatusParamUnknown;
        }
    } while(0);

    return status;
}
/******************************************************************/
//static inline
//hStatus get_timer(json_object *param_obj, json_object *janswer, hConfig *cfg)
//{
//    hStatus status = hStatusParseError;

//    do {
//        char *str_param = (char *) json_object_get_string(param_obj);
//        if (!str_param) {
//            break;
//        }

//        if (!strncmp(str_param, "all", strlen(str_param))) {
//            json_object_object_add(janswer, "light_on", json_object_new_int(cfg->sched_light_on));
//            json_object_object_add(janswer, "light_off", json_object_new_int(cfg->sched_light_off));
//            json_object_object_add(janswer, "pump_on", json_object_new_int(cfg->sched_pump_on));
//            json_object_object_add(janswer, "pump_off", json_object_new_int(cfg->sched_pump_off));
//            json_object_object_add(janswer, "pump_night", json_object_new_int(cfg->sched_pump_night));
//            status = hStatusOk;
//        }
//        else if (!strncmp(str_param, "light_on", strlen(str_param))) {
//            json_object_object_add(janswer, "light_on", json_object_new_int(cfg->sched_light_on));
//            status = hStatusOk;
//        }
//        else if (!strncmp(str_param, "light_off", strlen(str_param))) {
//            json_object_object_add(janswer, "light_off", json_object_new_int(cfg->sched_light_off));
//            status = hStatusOk;
//        }
//        else if (!strncmp(str_param, "pump_on", strlen(str_param))) {
//            json_object_object_add(janswer, "pump_on", json_object_new_int(cfg->sched_pump_on));
//            status = hStatusOk;
//        }
//        else if (!strncmp(str_param, "pump_off", strlen(str_param))) {
//            json_object_object_add(janswer, "pump_off", json_object_new_int(cfg->sched_pump_off));
//            status = hStatusOk;
//        }
//        else if (!strncmp(str_param, "pump_night", strlen(str_param))) {
//            json_object_object_add(janswer, "pump_night", json_object_new_int(cfg->sched_pump_night));
//            status = hStatusOk;
//        }
//        else {
//            status = hStatusParamUnknown;
//        }
//    } while(0);

//    return status;
//}
/******************************************************************/
static inline
hStatus parse_command(char *buff, hContainer *container)
{
    hStatus status = hStatusParseError;

    json_object *janswer = json_object_new_object();
    json_object_object_add(janswer, "error", json_object_new_int(status));
    json_object *error_obj = json_object_object_get(janswer, "error");
    struct json_object *request = json_tokener_parse(buff);

    do {
        if (!janswer || !error_obj || !request) {
            status = hStatusJsonObjCreateError;
            break;
        }

        json_object *command_obj = json_object_object_get(request, "command");
        json_object *param_obj = json_object_object_get(request, "param");
        char *str_command = (char *) json_object_get_string(command_obj);

        if (!command_obj || !param_obj || !str_command) {
            status = hStatusRequestFormatError;
            break;
        }

        if (!strncmp(str_command, COMMAND_GET_CFG, strlen(str_command))) {
            status = get_config(param_obj, janswer, container->config);
        }
        else if (!strncmp(str_command, COMMAND_SET_CFG, strlen(str_command))) {
            status = set_config(param_obj, container);
            if (isOk(status)) {
                container->is_config_changed = true;
            }
        }
        else if (!strncmp(str_command, COMMAND_GET_SYS_STATE, strlen(str_command))) {
            status = get_sys_state(param_obj, janswer, container->state);
        }
        else if (!strncmp(str_command, COMMAND_UPDATE_DATETIME, strlen(str_command))) {
            status = update_datettime();
        }
        else if (!strncmp(str_command, COMMAND_SYSTEM_START, strlen(str_command))) {
            status = system_start();
        }
        else if (!strncmp(str_command, COMMAND_SYSTEM_STOP, strlen(str_command))) {
            status = system_stop();
        }
        else {
            status = hStatusCommandUnknown;
        }
    } while(0);

    if (janswer && error_obj && buff) {
        json_object_set_int(error_obj, status);
        bzero(buff, BUFSIZ);
        strncpy(buff, json_object_to_json_string_ext(janswer, JSON_C_TO_STRING_SPACED), BUFSIZ);
    }

    return status;
}
/******************************************************************/
static inline
hStatus prepare_error_answer(char *buff, hStatus status)
{
    json_object *janswer = json_object_new_object();
    if (janswer && buff) {
        json_object_object_add(janswer, "error", json_object_new_int(status));
        bzero(buff, BUFSIZ);
        strncpy(buff, json_object_to_json_string_ext(janswer, JSON_C_TO_STRING_SPACED), BUFSIZ);
    }

    return status;
}
/******************************************************************/
