#ifndef __DSO_H__ 
#define __DSO_H__

#include "list.h"

#define MODULE_OK  0
#define MODULE_ERR 1

#define MAGIC_MAJOR_NUMBER 20140814
#define MAGIC_MINOR_NUMBER 0


#define STANDARD_MODULE_STUFF MAGIC_MAJOR_NUMBER, \
                              MAGIC_MINOR_NUMBER, \
                              __FILE__

typedef struct Module{
    int     version;        //主版本号
    int     minor_version;  //次版本号
    const char *name;       //模块名称
    void (*init)(struct Module *); //初始化函数指针
    int (*handle)(void *);  //入口函数指针
    struct list_head head;
}Module;

typedef struct Module_queue{
    int bufSize;
    struct list_head head;
}Module_queue;

extern Module_queue modules_pre_surl;
extern Module_queue modules_post_header;
extern Module_queue modules_post_html;

#define SPIDER_ADD_MODULE_PRE_SURL(module) do {\
     push_module_queue(&modules_pre_surl, &(module->head));\
} while(0)

#define SPIDER_ADD_MODULE_POST_HEADER(module) do {\
     push_module_queue(&modules_post_header, &(module->head));\
} while(0)


#define SPIDER_ADD_MODULE_POST_HTML(module) do {\
     push_module_queue(&modules_post_html, &(module->head));\
} while(0)


Module * dso_load(const char *, const char *);

void push_module_queue(Module_queue *, struct list_head *);

void module_init();

#endif