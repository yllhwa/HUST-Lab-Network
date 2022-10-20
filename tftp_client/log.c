//
// Created by yll20 on 2022/10/19.
//

#include "log.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


#define MIN_LOG_LEVEL LOG_DEBUG

FILE *log_file;

void init_log() {
    log_file = fopen("log.txt", "a");
}

void _Log(enum LOG_LEVEL level, char *msg) {
    if (level < MIN_LOG_LEVEL) {
        return;
    }
    // 获取当前时间
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char *time_str = asctime(local);
    // 去掉换行符
    time_str[strlen(time_str) - 1] = '\0';
    // 打印日志
    fprintf(log_file, "[%s] %s %s\n",
            level == LOG_DEBUG ? "DEBUG" :
            level == LOG_INFO ? "INFO" :
            level == LOG_WARN ? "WARN" :
            level == LOG_ERROR ? "ERROR":
            level == LOG_SPEED ? "SPEED" : "UNKNOWN",
            time_str,
            msg);
//    fflush(log_file);
}

void Log(enum LOG_LEVEL level, const char* format, ...)
{
    char* buffer = NULL;
    va_list args;
    va_start(args, format);
    int len = _vscprintf(format, args) + 1;
    buffer = (char*)malloc(len * sizeof(char));
    vsprintf_s(buffer, len, format, args);
    va_end(args);
    _Log(level, buffer);
    free(buffer);
}