#ifndef _THREADS_H__
#define _THREADS_H__

#include <pthread.h>

extern int g_cur_thread_num;

int create_thread(void *(*func) (void *), void *arg, pthread_t * thread, pthread_attr_t * pAttr);

//void start_thread();

void end_thread();


#endif