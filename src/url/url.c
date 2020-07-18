#include "hashmap.h"
#include "url.h"

static queue surl_queue;

static queue ourl_queue;

static HashMap host_ip_map;

void initQueue()
{
    surl_queue.bufSize = 0;
    INIT_LIST_HEAD(&surl_queue.head);
    pthread_cond_init(&surl_queue.cond);
    pthread_mutex_init(&surl_queue.bufs_mutex);
    pthread_mutex_init(&surl_queue.cond_mutex);
      
    ourl_queue.bufSize = 0;
    INIT_LIST_HEAD(&ourl_queue.head);
    pthread_cond_init(&ourl_queue.cond);
    pthread_mutex_init(&ourl_queue.bufs_mutex);
    pthread_mutex_init(&ourl_queue.cond_mutex);
}

/* 初始化 */
void 
initUrl()
{
    initQueue();
}

static void 
push_queue(queue *p, struct list_head *e)
{
    if(p == NULL || e == NULL)
        return;
    
    list_add_tail(&p->head, e);
    p->bufSize++;

}

static void 
pop_queue(queue *p, struct list_head **e)
{
    if(p == NULL || e == NULL)
        return;
    
    if(list_empty(&p->head))
    {
        *e == NULL;
        return;
    }

    *e = p->head.next;
    list_del(*e);
    p->bufSize--;
}


/* 解析域名*/
void *
urlparser(void *arg)
{
    Surl *url;
    Url *ourl
    HashMapIterator iterator ;

    while(1)
    {
        pthread_mutex_lock(&surl_queue.cond_mutex);
        
        while(surl_queue.bufSize == 0)
            pthread_cond_wait(&surl_queue.cond, &surl_queue.cond_mutex)

        //取出surl队列

        //surl结构转化为url

        //map中查找主机ip

        //map中不存在主机ip,解析dns
        if()
        {

        }
        else
        {   //map中存在,送入ourl队列中
            
        }
        
    }

    return NULL;
}