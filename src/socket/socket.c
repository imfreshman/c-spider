#include "socket.h"
#include "log.h"
#include "common.h"
#include "cstring.h"
#include "dso.h"
#include "list.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>


static const char *HREF_PATTERN = "href=\"\\s*\\([^ >\"]*\\)\\s*\"";

static Header *parse_header(char *header);
static int header_postcheck(Header *header);

int build_connnet(int *fd, char *ip, int port)
{
    struct sockaddr_in server_addr;

    bzero(&server_addr, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(!inet_aton(ip, &(server_addr.sin_addr)))
    {
        SPIDER_LOG(SPIDER_LEVEL_DEBUG,"Ip translate failed. error:[%s]", strerror(errno));
        return -1;
    }

    if((*fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Create socket failed. error:[%s]", strerror(errno));
        return -1;
    }

    if(connect(*fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0 )
    {
        close(*fd);
        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Connect failed. error:[%s]", strerror(errno));
        return -1;
    }

    return 0;
}


void set_nonblocking(int fd)
{
    int flag;

    if((flag = fcntl(fd, F_GETFL)) < 0)
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "fcntl getfl fail");

    flag |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flag) < 0)
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "fcntl setfl fail");
}


int send_request(int fd, void *arg)
{
    int need, begin, n;
    char request[1024];
    Url *url;

    if(arg == NULL)
        return -1;

    url = (Url *)arg;

    memset(request, 0x00, sizeof(request));
    sprintf(request, "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Accept: */*\r\n"
            "Connection: Keep-Alive\r\n"
            "User-Agent: Mozilla/5.0 (compatible; Qteqpidspider/1.0;)\r\n"
            "Referer: %s\r\n\r\n", url->path, url->domain, url->domain);

    need = strlen(request);
    begin = 0;
    while(need)
    {
        n = write(fd, request + begin, need);
        if(n <= 0)
        {
            if(errno == EAGAIN)
            {
                //write buffer full, delay retry
                usleep(1000);
                continue;
            }
            SPIDER_LOG(SPIDER_LEVEL_WARN, "Thread %lu send ERROR: %d", pthread_self(), errno);
            //free(url);
            close(fd);
            return -1;
        }
        begin += n;
        need -= n;
    }
    return 0;
}

static Header* 
parse_header(char *header)
{
    int c = 0;
    char *p = NULL;
    char **sps = NULL;
    char *start = header;
    Header * h = (Header *)calloc(1, sizeof(Header));

    if((p = strstr(start, "\r\n")) != NULL)
    {
        *p = '\0';
        sps = cssplitlen(start, strlen(start), " ", 1,&c);
        if(c == 3)
        {
            h->status_code = atoi(sps[1]);
        }
        else
        {
            h->status_code = 600;
        }
        start = p + 2;
    }

    while((p = strstr(start, "\r\n")) != NULL)
    {
        *p = '\0';
        sps = cssplitlen(start, strlen(start), ":", 1, &c);
        if(c == 2)
        {
            if (strcasecmp(sps[0], "content-type") == 0) 
            {
                h->content_type = strdup(csstrim(sps[1], "\t\r\n"));
            }            
        }
        start = p + 2;
    }
    return h;
}

void * recv_response(void *arg)
{
    int i, n, trunc_head = 0, len = 0;
    char * body_ptr = NULL;
    regex_t re;
    evso_arg * narg = (evso_arg *)arg;
    Response *resp = (Response *)malloc(sizeof(Response));

    resp->header = NULL;
    resp->body = (char *)malloc(HTML_MAXLEN);
    resp->body_len = 0;
    resp->url = narg->url; 

    if(regcomp(&re, HREF_PATTERN, 0) != 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "compile regex error");
        return NULL;
    }
    
    SPIDER_LOG(SPIDER_LEVEL_INFO, "Crawling url: %s/%s", narg->url->domain, narg->url->path);

    while(1)
    {
        n = read(narg->fd, resp->body + len, 1024);
        //SPIDER_LOG(SPIDER_LEVEL_DEBUG, "read len: [%d]", n);
        if(n < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                usleep(100000);
                continue;  
            }
            SPIDER_LOG(SPIDER_LEVEL_WARN, "Read socket fail: %s", strerror(errno));
            break;
        }
        else if (n == 0)
        {
            /* finished reading */
            resp->body_len = len;
            if(resp->body_len > 0)
            {
                extract_url(&re, resp->body, narg->url);
            }

            /* deal resp->body */
            Module *pos, *n;
            list_for_each_entry_safe(pos, n, &modules_post_html.head, head)
            {
                pos->handle(resp);
            }
            break;
        }
        else
        {
            SPIDER_LOG(SPIDER_LEVEL_DEBUG, "read socket ok! len=%d", n);
            len += n;
            resp->body[len] = '\0';
            if(trunc_head == 0)
            {
                if((body_ptr = strstr(resp->body, "\r\n\r\n")) != NULL)
                {
                    SPIDER_LOG(SPIDER_LEVEL_DEBUG,"body:[%s]", body_ptr);
                    *(body_ptr+2) = '\0';
                    SPIDER_LOG(SPIDER_LEVEL_DEBUG,"body:[%s]", body_ptr);
                    resp->header = parse_header(resp->body);
                    if(header_postcheck(resp->header) == 0)
                        goto leave;

                    trunc_head = 1;

                    /* cover header */
                    body_ptr += 4;
                    for(i = 0; *body_ptr; i++)
                    {
                        resp->body[i] = *body_ptr;
                        body_ptr++;                        
                    }
                    resp->body[i] = '\0';
                    len = i;
                    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "resp body len[%d]", len);                    
                }
                continue;
            }
        }
        
    }
leave:
    close(narg->fd); /* close socket */
    free_url(narg->url); /* free Url object */
    regfree(&re); /* free regex object */
    /* free resp */
    free(resp->header->content_type);
    free(resp->header);
    free(resp->body);
    free(resp);

    end_thread();
    return NULL;
}


//
static int 
header_postcheck(Header *header)
{
    Module *pos, *n;
    list_for_each_entry_safe(pos, n, &modules_post_header.head, head)
    {
        if(pos->handle(header) != MODULE_OK)
            return 0;
    }    
    return 1;
}