#include "dso.h"
#include "common.h"
#include "log.h"
#include <dlfcn.h>

Module_queue modules_pre_surl;
Module_queue modules_post_header;
Module_queue modules_post_html;

void module_init()
{
    modules_pre_surl.bufSize = 0;
    modules_post_header.bufSize = 0;
    modules_post_html.bufSize = 0;

    INIT_LIST_HEAD(&modules_pre_surl.head);
    INIT_LIST_HEAD(&modules_post_header.head);
    INIT_LIST_HEAD(&modules_post_html.head);
}



Module * 
dso_load(const char * path, const char *name)
{
    void *rv;
    void *handle;
    Module *module;
    char npath[SPIDER_FILE_PATH_MAX];

    memset(npath, 0x00, sizeof(npath));
    sprintf(npath, "%s/%s.so", path, name);

    if((handle =dlopen(npath, RTLD_GLOBAL | RTLD_NOW)) == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Load module fail(dlopen): %s", dlerror());
    }

    if((rv = dlsym(handle, name)) == NULL)
    {
        dlclose(handle);
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Load module fail(dlsym): %s", dlerror());
    }

    module = (Module *)rv;
    module->init(module);

    return module;
}


void 
push_module_queue(Module_queue *p, struct list_head *e)
{
    if(p == NULL || e == NULL)
        return;
    
    list_add_tail(e, &p->head);
    p->bufSize++;
}