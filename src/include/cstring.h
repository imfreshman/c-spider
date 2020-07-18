#ifndef __CSTRING_H__
#define __CSTRING_H__

#include <sys/types.h>

#define CS_MAX_PREALLOC (1024 * 1024)

struct cstring{
    int len;
    int free;
    char buf[];
};

static inline size_t 
cslen(char *s)
{
    struct cstring *cs = (void *)(s - (sizeof(struct cstring)));
    return cs->len;
}

static inline size_t 
csavail(char *s)
{
    struct cstring *cs = (void *)(s - (sizeof(struct cstring)));
    return cs->free;
}

char *csnewlen(const void *, size_t );
char *csempty(void);
char *cscat(char *, const char *);
char **cssplitlen(char *, int, const char *, int, int *);
char *csstrim(char *, const char *);
char **cssplitargs(const char *, int *);

void cstolower(char *);

void csfree(char *s);
void csfreesplitres(char **, int);


#endif