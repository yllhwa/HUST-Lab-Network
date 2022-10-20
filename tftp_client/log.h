//
// Created by yll20 on 2022/10/19.
//

#ifndef TFTP_CLIENT_LOG_H
#define TFTP_CLIENT_LOG_H

enum LOG_LEVEL {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_SPEED,
};

void init_log();
void Log(enum LOG_LEVEL level, const char* format, ...);

#endif //TFTP_CLIENT_LOG_H
