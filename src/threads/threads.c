#include "threads.h"
#include "common.h"
#include "log.h"



/* 当前线程数 */
int g_cur_thread_num = 0;

/* 给当前线程数g_cur_thread_num的值上锁 */
pthread_mutex_t gctn_lock = PTHREAD_MUTEX_INITIALIZER;

int create_thread(void *(*func) (void *), void *arg, pthread_t * thread, pthread_attr_t * pAttr)
{
    pthread_attr_t attr;
    pthread_t pt;

    if (pAttr == NULL) {
        pAttr = &attr;
        pthread_attr_init(pAttr);
        pthread_attr_setstacksize(pAttr, 1024*1024);
        pthread_attr_setdetachstate(pAttr, PTHREAD_CREATE_DETACHED);
    }

    if (thread == NULL)
        thread = &pt;

    int rv = pthread_create(thread, pAttr, func, arg);
    pthread_attr_destroy(pAttr);
    return rv;
}

void end_thread()
{
    pthread_mutex_lock(&gctn_lock);	
    int left = g_conf.max_job_num - (--g_cur_thread_num);
    if (left == 1) {
        /* can start one thread */
        attach_epoll_task();
    } else if (left > 1) {
        /* can start two thread */
        attach_epoll_task();
        attach_epoll_task();
    } else {
        /* have reached g_conf->max_job_num , do nothing */
    }
    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "End Thread %lu, cur_thread_num=%d", pthread_self(), g_cur_thread_num);
    pthread_mutex_unlock(&gctn_lock);	
}