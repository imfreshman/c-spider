#ifndef __LOG_H__
#define __LOG_H__

#include "spider.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>



/* 日志级别 */
#define SPIDER_LEVEL_DEBUG 0
#define SPIDER_LEVEL_INFO  1
#define SPIDER_LEVEL_WARN  2
#define SPIDER_LEVEL_ERROR 3
#define SPIDER_LEVEL_CRIT  4 

static const char * LOG_STR[] = { 
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRIT"
};


#define SPIDER_LOG(level, format, ...) do{  \
    if(level >= g_conf.log_level){  \
        time_t now = time(NULL);    \
        char buf[32];   \
        strftime(buf, sizeof(buf), "%Y%m%d %H:%M:%S", localtime(&now));    \
        fprintf(stdout,  "[%s] [%s] " format " \n", buf, LOG_STR[level], ##__VA_ARGS__);    \
        fflush(stdout);\
    }\
    if(level == SPIDER_LEVEL_ERROR){ \
        exit(-1);\
    }   \
}while(0);


#endif