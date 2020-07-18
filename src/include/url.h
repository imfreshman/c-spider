#ifndef __URL_H__
#define __URL_H__

#include "list.h"
#include <pthread.h>


/* 维护url原始字符串 */
typedef struct Surl{
    char  *url;
    int    level;   //url爬取深度
    int    type;    //抓取类型
    struct list_head head;
}Surl;

/* 解析url*/
typedef struct Url{
    char *domain;   //域名
    char *path;     //路径
    int  port;      //端口
    char *ip;       //ip
    int  level;     //深度
    struct list_head head;
}Url;


typedef struct evso_arg {
    int     fd;
    Url     *url;
} evso_arg;


typedef struct queue
{
    int bufSize;
    struct list_head head;
    pthread_cond_t cond;
    pthread_mutex_t bufs_mutex;
    pthread_mutex_t cond_mutex;
}queue;


void initUrl();

void * urlparser(void * arg);


















#endif