#include "dso.h"
#include "socket.h"
#include "spider.h"

static int 
handler(void *data)
{
    Header *h = (Header *)data;
    int i = 0;

    if (h->status_code < 200 || h->status_code >= 300)
        return MODULE_ERR;

    if (h->content_type != NULL)
    {
        if (strstr(h->content_type, "text/html") != NULL)
            return MODULE_OK;
        params *pos, *n;
        list_for_each_entry_safe(pos, n, &g_conf.accept_types.head, head)
        {
            if(strstr(h->content_type, pos->elem) != NULL)
                return MODULE_OK;
        }
        return MODULE_ERR;    
    }
    return MODULE_OK;    
}

static void
init (Module *mod)
{
    SPIDER_ADD_MODULE_POST_HEADER(mod);
}

Module headerfilter = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};