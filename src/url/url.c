#include "hashmap.h"
#include "log.h"
#include "url.h"
#include "bloomfilter.h"
#include "threads.h"
#include "epoll.h"

queue surl_queue;

queue ourl_queue;

static HashMap host_ip_map;

static int is_bin_url(char *url);

static void dns_callback(int result, char type, int count, int ttl, void *addresses, void *arg);


static void 
initHostIpMap()
{
    host_ip_map = creatHashMap(NULL, NULL);
    if(host_ip_map == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "creat host_ip_map error :%d", errno);
        exit(1);
    }
}

static void 
initQueue()
{
    surl_queue.bufSize = 0;
    INIT_LIST_HEAD(&surl_queue.head);
    pthread_cond_init(&surl_queue.cond, NULL);
    pthread_mutex_init(&surl_queue.bufs_mutex, NULL);
    pthread_mutex_init(&surl_queue.cond_mutex, NULL);
      
    ourl_queue.bufSize = 0;
    INIT_LIST_HEAD(&ourl_queue.head);
    pthread_cond_init(&ourl_queue.cond, NULL);
    pthread_mutex_init(&ourl_queue.bufs_mutex, NULL);
    pthread_mutex_init(&ourl_queue.cond_mutex, NULL);

}

/* 初始化 */
void 
initUrl()
{
    initQueue();
    initHostIpMap();
}

void 
push_queue(queue *p, struct list_head *e)
{
    if(p == NULL || e == NULL)
        return;
    pthread_mutex_lock(&p->bufs_mutex);
    list_add_tail(e, &p->head);
    p->bufSize++;
    pthread_mutex_unlock(&p->bufs_mutex);
    if(p->bufSize == 1)
        pthread_cond_signal(&p->cond);
    
}

void 
pop_queue(queue *p, struct list_head **e)
{
    if(p == NULL || e == NULL)
        return;
    
    if(list_empty(&p->head))
    {
        *e == NULL;
        return;
    }

    pthread_mutex_lock(&p->bufs_mutex);
    *e = p->head.next;
    list_del(*e);
    p->bufSize--;
    pthread_mutex_unlock(&p->bufs_mutex);
}

int 
is_queue_empty(queue *p)
{
    int bufSize;
    pthread_mutex_lock(&p->bufs_mutex);
    bufSize = p->bufSize;
    pthread_mutex_unlock(&p->bufs_mutex);
    if(bufSize == 0)
        return 1;
    return 0;
}

static Url *
surl2Ourl(Surl *surl)
{
    Url *ourl;
    char *p;

    if(surl == NULL)
        return NULL;
    
    ourl = (Url*)calloc(1, sizeof(Url));
    //path
    p = strchr(surl->url, '/');
    if(p == NULL)
    {
        ourl->domain = surl->url;
        ourl->path = surl->url + strlen(surl->url);
    }
    else
    {
        *p = '\0';
        ourl->domain = surl->url;
        ourl->path = p + 1;
    }
    //port
    p = strchr(surl->url, ':');
    if( p != NULL )
    {
        *p = '\0';
        ourl->port = atoi(p+1);
        if(ourl->port == 0)
            ourl->port = 80;
    }
    else
        ourl->port = 80;
    
    // level
    ourl->level = surl->level;
    return ourl;
}


/* 解析域名*/
void *
urlparser(void *arg)
{
    Surl *surl, *pos, *n;
    Url *ourl;

    while(1)
    {
        pthread_mutex_lock(&surl_queue.cond_mutex);
        
        while(surl_queue.bufSize == 0)
            pthread_cond_wait(&surl_queue.cond, &surl_queue.cond_mutex);

        pthread_mutex_unlock(&surl_queue.cond_mutex);

        //取出surl队列
        pthread_mutex_lock(&surl_queue.bufs_mutex);
        list_for_each_entry_safe(pos, n, &surl_queue.head, head)
        {
            //surl结构转化为url
            ourl = surl2Ourl(pos);
            if(ourl == NULL)
            {
                SPIDER_LOG(SPIDER_LEVEL_WARN, "surl2Ourl failed");
                break;
            }
            
            //删除节点
            list_del(&pos->head);
            surl_queue.bufSize--;

            SPIDER_LOG(SPIDER_LEVEL_DEBUG, "ourl:[%s]", ourl->domain);

            //map中查找主机ip
            //map中不存在主机ip,解析dns
            if(host_ip_map->exists(host_ip_map, ourl->domain) == False)
            {
                //dns解析,待完成
                struct event_base *base = event_init();
                evdns_init();
                //evdns_base_resolve_ipv4(base, ourl->domain, 0, dns_callback, ourl);
                evdns_resolve_ipv4(ourl->domain, 0, dns_callback, ourl);
                event_dispatch();
                event_base_free(base);
            }
            else
            {   
                //map中存在,送入ourl队列中
                ourl->ip = strdup((char*)(host_ip_map->get(host_ip_map, ourl->domain)));
                push_queue(&ourl_queue, &ourl->head);
                if(g_cur_thread_num == 0)
                    attach_epoll_task();
            }
        }
        pthread_mutex_unlock(&surl_queue.bufs_mutex);
        
    }

    return NULL;
}

int addUrlSeed(char *seed, int level)
{
    Surl *surl;

    surl = (Surl *)malloc(sizeof(Surl));
    surl->url = urlNormalized(seed);
    surl->level = level;
    surl->type = TYPE_HTML;
    if(surl->url != NULL)
    {
        push_queue(&surl_queue, &surl->head);
        return 0;
    }

    return -1;
}

char *
urlNormalized(char *url)
{
    int len,vlen; 
    char *tmp;

    if(url == NULL)
        return NULL;

    len = strlen(url);

    while(len && isspace(url[len - 1]))
        len--;
    
    url[len] = '\0';

    if(len == 0)
        return NULL;
     
    /* remove http(s):// */
    if(len > 7 && strncmp(url, "http", 4) == 0)
    {
        vlen = 7;
        if(url[4] == 's') /* https */
            vlen++;
        len = len - vlen;
        tmp = (char *)malloc(len + 1);
        strncpy(tmp, url+vlen, len);
        tmp[len] = '\0';
        memset(url, 0x00, strlen(url));
        strncpy(url, tmp, len);
        free(tmp);
    }


    if(url[len - 1 ] == '/')
        url[--len] = '\0';

    if(len > MAX_LINK_LEN)
        return NULL;
    
    return url;
    
}

/*
 * 返回最后找到的链接的下一个下标,如果没找到返回 0;
 */
int 
extract_url(regex_t *re, char *str, Url *ourl)
{
    const size_t nmatch = 2;
    regmatch_t matchptr[nmatch];
    int len;
    char *p;

    p = str;
    while(regexec(re, p, nmatch, matchptr, 0) != REG_NOMATCH)
    {
        len = (matchptr[1].rm_eo - matchptr[1].rm_so);
        p = p + matchptr[1].rm_so;
        char *tmp = (char *)calloc(len+1, 1);
        strncpy(tmp, p, len);
        tmp[len] = '\0';
        p = p + len + (matchptr[0].rm_eo - matchptr[1].rm_eo);

        /* exclude binary file */
        if (is_bin_url(tmp)) {
            free(tmp);
            continue;
        }

        char *url = attach_domain(tmp, ourl->domain);
        if(url != NULL)
        {
            SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Find a url: %s", url);
            Surl * surl = (Surl *)malloc(sizeof(Surl));
            surl->level = ourl->level + 1;
            surl->type = TYPE_HTML;

            /* normalize url */
            if ((surl->url = urlNormalized(url)) == NULL) 
            {
                SPIDER_LOG(SPIDER_LEVEL_WARN, "Normalize url fail");
                free(surl);
                continue;
            }

            if(iscrawled(surl->url))
            {   
                SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Have seen this url: %s", surl->url);
                free(surl->url);
                free(surl);
                continue;                
            }   
            else
            {
                push_queue(&surl_queue, &surl->head);
            }
                                
        }

    }
    return (p-str);
}

/* if url refer to binary file
 * image: jpg|jpeg|gif|png|ico|bmp
 * flash: swf
 */
static char * BIN_SUFFIXES = ".jpg.jpeg.gif.png.ico.bmp.swf";
static int is_bin_url(char *url)
{
    char *p = NULL;
    if ((p = strrchr(url, '.')) != NULL) {
        if (strstr(BIN_SUFFIXES, p) == NULL)
            return 0;
        else
            return 1;
    }
    return 0;
}

char * 
attach_domain(char *url, const char *domain)
{
    if (url == NULL)
        return NULL;

    if (strncmp(url, "http", 4) == 0) 
    {
        return url;

    } 
    else if (*url == '/') 
    {
        int i;
        int ulen = strlen(url);
        int dlen = strlen(domain);
        char *tmp = (char *)malloc(ulen+dlen+1);
        for (i = 0; i < dlen; i++)
            tmp[i] = domain[i];
        for (i = 0; i < ulen; i++)
            tmp[i+dlen] = url[i];
        tmp[ulen+dlen] = '\0';
        free(url);
        return tmp;

    } 
    else 
    {
        //do nothing
        free(url);
        return NULL;
    }
}


int 
iscrawled(char * url) 
{
    return search(url); /* use bloom filter algorithm */
}

char * 
url2fn(const Url * url)
{
    int i = 0;
    int l1 = strlen(url->domain);
    int l2 = strlen(url->path);
    char *fn = (char *)malloc(l1+l2+2);

    for (i = 0; i < l1; i++)
        fn[i] = url->domain[i];

    fn[l1++] = '_';

    for (i = 0; i < l2; i++)
        fn[l1+i] = (url->path[i] == '/' ? '_' : url->path[i]);

    fn[l1+l2] = '\0';

    return fn;
}

void free_url(Url * ourl)
{
    //free(ourl->domain);
    //free(ourl->path);
    free(ourl->ip);
    free(ourl);
}




static void
dns_callback(int result, char type, int count, int ttl, void *addresses, void *arg)
{
    Url *ourl = (Url *)arg;
    struct in_addr *addrs = (struct in_addr *)addresses;

    if(result != DNS_ERR_NONE || count == 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN, "Dns resolve fail: %s", ourl->domain);
    }
    else
    {
        char * ip = inet_ntoa(addrs[0]);
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Dns resolve OK: %s -> %s", ourl->domain, ip);        
        host_ip_map->put(host_ip_map, ourl->domain, strdup(ip));
        ourl->ip = strdup(ip);
        push_queue(&ourl_queue, &ourl->head);
        if(g_cur_thread_num == 0)
            attach_epoll_task();
    }
    event_loopexit(NULL);
    
}