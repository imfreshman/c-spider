#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "list.h"


#define INF 0x7FFFFFFF
#define DEFAULT_CONFIG_FILE  "spider.conf"


typedef struct params
{
    char *elem;
    struct list_head head;
}params;


typedef struct params_queue
{
    int bufSize;
    struct list_head head;
}params_queue;


void init_params_queue(params_queue *);
void push_params_queue(params_queue *, struct list_head *);
void pop_params_queue(params_queue *, struct list_head **);


typedef struct spiderConfig {
    int              max_job_num;           //最大任务数
    char            *seeds;                 //种子
    char            *include_prefixes; 
    char            *exclude_prefixes; 
    char            *logfile;               //日志文件
    int              log_level;             //日志等级
    int              max_depth;             //深度
    int              make_hostdir;          
    int              stat_interval;

    char *           module_path;           //模块路径
    params_queue           modules;               
    params_queue           accept_types;          //抓取类型
}spiderConfig;

void initConfig(spiderConfig *);

void loadConfigFromString(char *);

void loadConfig(const char *);

int getConfAbsolutePath(char *);

#endif