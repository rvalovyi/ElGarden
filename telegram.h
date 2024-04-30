#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "common.h"
#include "json-helper.h"
#include "socket-helper.h"

/******************************************************************/
static inline
void *telegram_thread(void *vargp)
{
    bool is_running_ = true;
    char buff[BUFSIZ];
    int ssock, csock;
    struct timespec t;
    hContainer *container = (hContainer *)vargp;
    hStatus status = hStatusOk;

    status = createSocket(PORT, &ssock);
    if (isOk(status)) {
        hLog(LOG_INFO, "Server listening on port %d", PORT);
        while(is_running_) {
            bzero(buff, BUFSIZ);
            status = wait_client(ssock, buff, BUFSIZ, &csock, POLL_TIMEOUT_MS);
            if (isOk(status)) {
                LOCK(container->mutex);
                for (uint32_t i = 0; i < TIME_1S_IN_NS / TIME_20MS_IN_NS && container->is_config_changed; i++) {
                    if (!clock_gettime(CLOCK_REALTIME, &t)) {
                        t.tv_nsec += TIME_20MS_IN_NS;
                        if (t.tv_nsec > TIME_NS_MAX) {
                            t.tv_nsec -= TIME_NS_MAX;
                            t.tv_sec++;
                        }
                        int res = pthread_cond_timedwait(&container->cond, &container->mutex, &t);
                        if (res < 0 && res != ETIMEDOUT) {
                            hLog(LOG_INFO, "pthread_cond_timedwait(%u) timeout res = %d", i, res);
                        }
                    }
                }

                if (!container->is_config_changed) {
                    status = parse_command(buff, container);
                }
                else {
                    status = hStatusBusy;
                }
                UNLOCK(container->mutex);

                if(!isOk(status)) {
                    prepare_error_answer(buff, status);
                }

                sendMsg(csock, buff, strlen(buff));

                closeSocket(csock);
            }

            if (!TRYLOCK(container->mutex)) {
                is_running_ = container->telegram_thread_state == THREAD_STATE_RUNNING;
                UNLOCK(container->mutex);
            }
        }
        closeSocket(ssock);
    }

    if (!is_running_) {
        hLog(LOG_INFO, "Telegram thread is terminated due to receipt of a termination signal");
    }
    else {
        hLog(LOG_WARNING, "Telegram thread is finished unexpected, status: %u, reason: %s",
             status, strerror(errno));
    }

    pthread_mutex_lock(&container->mutex);
    container->telegram_thread_state = THREAD_STATE_FINISHED;
    pthread_mutex_unlock(&container->mutex);

    return NULL;
}
/******************************************************************/
