#ifndef __LOGGING__H
#define __LOGGING__H

#include <stddef.h>

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;


void serial_init();

void log(log_level_t level, const char* message, ...);



#endif // !__LOGGING__H