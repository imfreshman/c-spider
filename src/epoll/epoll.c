#include "spider.h"
#include "common.h"
#include "log.h"
#include "url.h"
#include "socket.h"
#include "threads.h"
#include <sys/epoll.h>

int g_epfd;


void init_epoll()
{
    g_epfd = epoll_create(g_conf.max_job_num);

    if(g_epfd == -1)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Create epoll failed! [%s]", strerror(errno));
        exit(1);
    }
}


int attach_epoll_task()
{
    struct epoll_event ev;
    int sock_rv;
    int sockfd;
    Url *ourl;
    evso_arg *arg;
    struct list_head *e = NULL;

    pop_queue(&ourl_queue, &e);
    if(e == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Pop ourlqueue fail!");
        return -1;
    }
    ourl = list_entry(e, Url, head);

    /* connect socket and get sockfd */
    if(build_connnet(&sockfd, ourl->ip, ourl->port) < 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Build socket connect fail: %s", ourl->ip);
        return -1;
    }

    /* set nonblock */
    set_nonblocking(sockfd);

    if(send_request(sockfd, ourl) < 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Send socket request fail: %s", ourl->ip);
        free_url(ourl);
        return -1;
    }

    arg = (evso_arg *)calloc(1, sizeof(sizeof(evso_arg)));
    if(arg == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Evso_arg calloc fail. error: %s", strerror(errno));
        free_url(ourl);
        return -1;
    }

    arg->fd = sockfd;
    arg->url = ourl;
    ev.data.ptr = arg;
    ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(g_epfd, EPOLL_CTL_ADD, sockfd, &ev) == 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Attach an epoll event success! url[%s].",ourl->domain);
    }
    else
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Attach an epoll event fail! url[%s].",ourl->domain);
        free_url(ourl);
        return -1;
    }
    
    g_cur_thread_num++;
    return 0;
}

void epoll_proc()
{
    int ourl_num, try_num;
    int n, i;
    evso_arg * arg;
    struct epoll_event events[10];

    try_num = 1;
    /* waiting seed ourl ready */
    while(try_num < 8 && is_queue_empty(&ourl_queue))
    {
        usleep((10000 << try_num++));
    }

    /*
    if (try_num >= 8) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "NO ourl! DNS parse error?");
    }*/

    /* set ticker */

    /*
    ourl_num = 0;
    while(ourl_num++ < g_conf.max_job_num)
    {
        if(attach_epoll_task() < 0)
            break;
    }*/

    while(1)
    {
        n = epoll_wait(g_epfd, events, 10, 2000);
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "epoll:%d\n", n);

        if(n == -1)
            SPIDER_LOG(SPIDER_LEVEL_WARN, "epoll errno:%s\n",strerror(errno));
    
        if(n <= 0)
        {
            if (g_cur_thread_num <= 0 && is_queue_empty(&ourl_queue) && is_queue_empty(&surl_queue)) 
            {
                sleep(1);
                if (g_cur_thread_num <= 0 && is_queue_empty(&ourl_queue) && is_queue_empty(&surl_queue))
                    break;
            }            
        }


        for( i = 0 ; i < n; i++)
        {
            arg = (evso_arg *)(events[i].data.ptr);
            if((events[i].events & EPOLLERR) || //文件描述符发生错误
               (events[i].events & EPOLLHUP) || //文件描述符被挂断
               (!(events[i].events & EPOLLIN)))
               {
                    SPIDER_LOG(SPIDER_LEVEL_WARN, "epoll fail, close socket %d",arg->fd);
                    close(arg->fd);
                    continue;
               }
            /* del event */
            epoll_ctl(g_epfd, EPOLL_CTL_DEL, arg->fd, &events[i]); 

            SPIDER_LOG(SPIDER_LEVEL_DEBUG,"epoll:event=%d\n",events[i].events)
            
            /* create thread */
            create_thread(recv_response, arg, NULL, NULL); 
        }
    }
    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Task done!");
    close(g_epfd);
    return 0;
}


void start_epoll_server()
{
    init_epoll();
    epoll_proc();
}