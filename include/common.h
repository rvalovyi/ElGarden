#pragma once

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "logs.h"
#include "types.h"

/******************************************************************/
enum hydroStatus {
    hStatusOk = 0,
    hStatusUnknownError,
    hStatusInternalError,
    hStatusSocketError,
    hStatusBusy,
    hStatusNoData,
    hStatusThreadError,

    /* File errors */
    hStatusCfgDirCreateError = 30,
    hStatusFileNotFound,
    hStatusSaveFileError,
    hStatusTimeFuncError,

    /* JSON errors */
    hStatusJsonObjCreateError = 40,

    /* API errors */
    hStatusCommandUnknown = 50,
    hStatusRequestFormatError,
    hStatusParseError,
    hStatusCfgUnknown,
    hStatusParamUnknown,
    hStatusCfgLightError,
    hStatusCfgPumpError,
    hStatusCfgPHError,
    hStatusCfgECError,

    /* Timer errors */
    hStatusTimerCreateError = 70,
    hStatusTimerSigactionError,
    hStatusTimerSetTimeError,
    hStatusTimerDeleteError,

    /* Socket errors */
    hStatusSocketCreateError = 100,
    hStatusSocketBindError,
    hStatusSocketSendError
};
/******************************************************************/
typedef enum hydroStatus hStatus;
/******************************************************************/
static inline
bool isOk(hStatus status) { return status == hStatusOk; }
/******************************************************************/
#define LOCK(__m)  pthread_mutex_lock(&__m)
#define TRYLOCK(__m)  pthread_mutex_trylock(&__m)
#define UNLOCK_CV(__c, __m)               \
    ({                                    \
    pthread_cond_signal(&__c);            \
    pthread_mutex_unlock(&__m);           \
    })
#define UNLOCK(__m) pthread_mutex_unlock(&__m);
#define UNUSED(x) (void)(x)
/******************************************************************/
static inline
hStatus isDirExists(const char *path)
{
    struct stat st = {0};
    if (stat(path, &st) == 0 || mkdir(path, 0700) == 0) {
        return 0;
    }
    hLog(LOG_ERR, "%s error: %s : %s", __func__, strerror(errno), path);
    return hStatusCfgDirCreateError;
}
/******************************************************************/
static inline
hStatus isFileExists(const char *path)
{
    struct stat st = {0};

    if (stat(path, &st) == 0) {
        return 0;
    }
    hLog(LOG_ERR, "%s error: %s", __func__, strerror(errno));
    return hStatusFileNotFound;
}
/******************************************************************/
enum datetime_type{
    DATETIME_TYPEOUT_DATE,
    DATETIME_TYPEOUT_TIME,
    DATETIME_TYPEOUT_BIN,
};
/******************************************************************/
static inline
hStatus get_datettime(void *buff, size_t sz, time_t add_time, enum datetime_type type)
{
    hStatus status = hStatusOk;
    hDateTime dtime_;

    do {
        if (!buff) {
            hLog(LOG_ERR, "Argument unexpected");
            status = hStatusInternalError;
            break;
        }

        time_t rawtime = time(NULL) + add_time;
        if (rawtime % 60 == 59) {
            rawtime++;
        }
        if (rawtime == -1) {
            hLog(LOG_ERR, "The time() function failed: %s", strerror(errno));
            status = hStatusTimeFuncError;
            break;
        }

//        /* DEBUG Start*/
//        struct tm dbgTime;
//        static time_t dbgTime_t = 0;
//        dbgTime.tm_isdst = -1;
//        dbgTime.tm_mday = 27;
//        dbgTime.tm_mon = 4 - 1;
//        dbgTime.tm_year = 2024 - 1900;
//        dbgTime.tm_hour = 18;
//        dbgTime.tm_min = 56;
//        dbgTime.tm_sec = 30;
//        if (!dbgTime_t) dbgTime_t = rawtime - mktime(&dbgTime);
//        rawtime = rawtime - dbgTime_t;
//        /* DEBUG End*/

        struct tm *ptm = localtime(&rawtime);
        if (ptm == NULL) {
            hLog(LOG_ERR, "The localtime() function failed: %s", strerror(errno));
            status = hStatusTimeFuncError;
            break;
        }

        dtime_.tm_mday = ptm->tm_mday;
        dtime_.tm_mon = ptm->tm_mon + 1;
        dtime_.tm_year =ptm->tm_year + 1900;
        dtime_.tm_hour = ptm->tm_hour;
        dtime_.tm_min = ptm->tm_min;
        dtime_.tm_sec = ptm->tm_sec;

        switch (type) {
        case DATETIME_TYPEOUT_DATE:
            snprintf(buff, sz, "%02d.%02d.%02d",
                     dtime_.tm_mday,
                     dtime_.tm_mon,
                     dtime_.tm_year);
            break;
        case DATETIME_TYPEOUT_TIME:
            snprintf(buff, sz, "%02d:%02d:%02d",
                     dtime_.tm_hour,
                     dtime_.tm_min,
                     dtime_.tm_sec);
            break;
        case DATETIME_TYPEOUT_BIN:
            memcpy(buff, &dtime_, sizeof(dtime_));
            break;
        default:
            hLog(LOG_ERR, "Date-time type unexpected");
            status = hStatusInternalError;
            break;
        }
    } while(0);

    return status;
}
/******************************************************************/
/*
                        long long  start;
                        long long  stop;
                        start = current_timestamp();
                        stop = current_timestamp();
                        hLog(LOG_DEBUG, "Thread: %lld", stop - start);
*/
long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}
/******************************************************************/
