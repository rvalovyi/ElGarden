#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <netdb.h>
#include <poll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logs.h"
#include "common.h"
#include "defines.h"

/******************************************************************/
static inline
hStatus createSocket(int port, int *ssock)
{
    hStatus status = hStatusOk;
    int sock;
    int opt = 1;
    struct sockaddr_in server;

    do {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            hLog(LOG_ERR, "Socket creation failed: %s", strerror(errno));
            status = hStatusSocketCreateError;
            break;
        }
        hLog(LOG_DEBUG, "Socket created");

        if (setsockopt(sock, SOL_SOCKET,
                       SO_REUSEADDR | SO_REUSEPORT, &opt,
                       sizeof(opt))) {
            hLog(LOG_ERR, "Socket set options failed: %s", strerror(errno));
            status = hStatusSocketCreateError;
            break;
        }

        bzero((char *) &server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);
        if (bind(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
            hLog(LOG_ERR, "Bind failed: %s", strerror(errno));
            status = hStatusSocketBindError;
            break;
        }
        hLog(LOG_DEBUG, "Bind done");

        listen(sock , 3);
        *ssock = sock;
    } while(0);

    return status;
}
/******************************************************************/
static inline
void closeSocket(int sock)
{
    close(sock);
    hLog(LOG_DEBUG, "----------------------------");
    return;
}
/******************************************************************/
static inline
hStatus sendMsg(int sock, void* msg, uint32_t msgsize)
{
    hStatus status = hStatusOk;
    do {
        int res = write(sock, msg, msgsize);
        if (res < 0) {
            hLog(LOG_ERR, "Can't send message: %s", strerror(errno));
            closeSocket(sock);
            status = hStatusSocketSendError;
        }
        hLog(LOG_DEBUG, "Message sent (%d bytes): '%s'\n", res, msg);
    } while(0);

    return status;
}
/******************************************************************/
static inline
hStatus wait_client(int server_socket, char *buff, int sz, int *csock, int timeout_ms)
{
    hStatus status = hStatusNoData;
    static struct pollfd pollfds[1];

    pollfds[0].fd = server_socket;
    pollfds[0].events = POLLIN | POLLPRI;

    int poll_result = poll(pollfds, 1, timeout_ms);
    while (poll_result > 0) {
        if (pollfds[0].revents & POLLIN) {
            struct sockaddr_in cliaddr;
            socklen_t addrlen = sizeof(cliaddr);
            *csock = accept(server_socket, (struct sockaddr *)&cliaddr, &addrlen);
            if (*csock < 0) {
                hLog(LOG_ERR, "accept() failed: %s", strerror(errno));
                status = hStatusSocketError;
                break;
            }
            hLog(LOG_INFO, "Accepted connection from %s", inet_ntoa(cliaddr.sin_addr));
            int rsz = read(*csock, buff, sz - 1);
            if (rsz > 0) {
                buff[rsz] = '\0';
                hLog(LOG_DEBUG, "Recv from client: %s\n", buff);
                status = hStatusOk;
            }
            else {
                closeSocket(*csock);
            }
        }
        break;
    }

    return status;
}
/******************************************************************/
