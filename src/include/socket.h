#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "url.h"

/* http header */
typedef struct Header{
    char    *content_type;
    int      status_code;
} Header;


typedef struct Response{
    Header *header;
    char *body;
    int body_len;

    Url *url;
} Response;


int build_connnet(int *, char *, int);

int send_request(int, void *);

void set_nonblocking(int);

void * recv_response(void *);



#endif
