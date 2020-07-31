#ifndef __URL_H__
#define __URL_H__

#include "list.h"
#include "common.h"
#include <pthread.h>
#include <event.h>
#include <evdns.h>
#include <regex.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>


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


extern queue surl_queue;

extern queue ourl_queue;

void initUrl();

void * urlparser(void * arg);

int addUrlSeed(char *, int);

char *urlNormalized(char *);

void push_queue(queue *, struct list_head *);

void pop_queue(queue *, struct list_head **);

int is_queue_empty(queue *);

int extract_url(regex_t *, char *, Url *);

char *attach_domain(char *, const char *);

int iscrawled(char *); 

char * url2fn(const Url *);

void free_url(Url *);














#endif