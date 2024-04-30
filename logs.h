#ifndef __LOGS_H
#define __LOGS_H

#include <syslog.h>
#include <stdarg.h>

/******************************************************************/
void hLog(int log_prio, const char* message, ...) {
    va_list args;
    va_start(args, message);
    vsyslog(log_prio, message, args);
    va_end(args);
}
/******************************************************************/

#endif /* __LOGS_H */
