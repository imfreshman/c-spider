#include "cstring.h"
#include "log.h"
#include <ctype.h>
#include <stdlib.h>

char *
csnewlen(const void *init, size_t initlen)
{
    struct cstring * cs;

    if(init)
    {
        cs = malloc(sizeof(struct cstring) + initlen + 1);
    }
    else
    {
        cs = calloc(sizeof(struct cstring) + initlen + 1, sizeof(char));
    }
    
    if( cs == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN,"csnewlen malloc failed");
        return NULL;
    }
        
    cs->len = initlen;

    cs->free = 0; //

    if(initlen && init)
        memcpy(cs->buf, init, initlen);
    cs->buf[initlen] = '\0';

    return (char *)cs->buf;
    
}

char *
csMakeRoomFor(char *s, size_t addlen)
{
    struct cstring *cs, *newcs;

    size_t free = csavail(s);

    size_t len, newlen;

    if(free > addlen)   return s;

    len = cslen(s);
    cs = (void*)(s - (sizeof(struct cstring)));

    newlen = len + addlen;

    if(newlen < CS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += CS_MAX_PREALLOC;

    newcs = realloc(cs, sizeof(struct cstring) + newlen + 1);

    if(newcs == NULL)
    {
        SPIDER_LOG(SPIDER_LEVEL_WARN,"csMakeRoomFor readlloc failed");
        return NULL;
    }

    newcs->free = newlen - len;

    return newcs->buf;
}

char *
csempty(void)
{
    return csnewlen("", 0);
}

char *
cscatlen(char *s, const char *t, size_t len)
{
    struct cstring *cs;

    size_t curlen = cslen(s);

    s = csMakeRoomFor(s, len);

    if(s == NULL) return NULL;

    cs = (void *)(s - (sizeof(struct cstring)));
    memcpy(s + curlen, t, len);

    cs->len = curlen + len;
    cs->free = cs->free - len;

    s[curlen + len] = '\0';

    return s;
} 

char *
cscat(char *s, const char *t)
{
    return cscatlen(s, t, strlen(t));
}

char **
cssplitlen(char *s, int len, const char *sep, int seplen, int *count)
{
    int elements = 0, slots = 5, start = 0, j;
    char **tokens;

    if(seplen < 1 || len < 0)
        return NULL;

    tokens = malloc(sizeof(char *) * slots);
    if(tokens == NULL)
        return NULL;

    if(len == 0)
    {
        *count = 0;
        return tokens;
    }

    for(j = 0; j < (len - (seplen - 1)); j++)
    {
        /*确保有足够的空间*/
        if(slots < elements + 2)
        {
            char **newtokens;

            slots *= 2;
            newtokens = realloc(tokens, sizeof(char *) * slots);
            if(newtokens == NULL)
                goto cleanup;
            tokens = newtokens;
        }
        if((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j, sep, seplen) == 0))
        {
            tokens[elements] = csnewlen(s + start, j - start);
            if(tokens[elements] == NULL)
                goto cleanup;
            elements++;
            start = j + seplen;
            j = j + seplen - 1;
        }
    }
    //最后一个字符添加
    tokens[elements] = csnewlen(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;
cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) free(tokens[i]);
        free(tokens);
        *count = 0;
        return NULL;
    }

}

char *
csstrim(char *s, const char *cset)
{
    struct cstring * cs = (void*)(s- (sizeof(struct cstring)));
    char *start, *end, *sp, *ep;
    size_t len;

    //设置和记录指针
    sp = start  = s;
    ep = end = s + cslen(s) - 1;

    //修剪
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep >= start && strchr(cset, *ep)) ep--;

    //计算trim后，剩余的字符串长度
    len = (sp > ep) ? 0 : ((ep - sp) + 1);

    if(cs->buf != sp) memmove(cs->buf, sp, len);

    //添加终结字符
    cs->buf[len] = '\0';

    //更新属性
    cs->free = cs->free + (cs->len - len);
    cs->len = len;

    return s;
}
char **
cssplitargs(const char *line, int *argc)
{
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1)
    {
        //跳过空白
        while(*p && isspace(*p)) p++;

        if(*p)
        {
            /* get a token */
            int inq = 0;
            int insq = 0;
            int done = 0;

            if( current == NULL)
                current = csempty();

            while(!done)
            {
                /*待完成*/
                if(inq)
                {

                }
                else if(insq)
                {

                }
                else
                {
                    switch(*p)
                    {
                        case '=':
                        case '\n':
                        case '\r':
                        case '\t':
                        case '\0':  
                            done = 1;
                            break;
                        case '"':
                            inq = 1;
                            break;
                        case '\'':
                            insq = 1;
                            break;
                        default:
                            current = cscatlen(current, p, 1);
                            break;
                    }
                }
                if(*p) p++;               
            }

            /* add the token to the vector */
            vector  = realloc(vector, ((*argc) + 1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        }
        else
        {
            if(vector == NULL)  
                vector = malloc(sizeof(void*));
            
            return vector;
        }
        
    }
err:
    while((*argc)--)
        free(vector[*argc]);
    free(vector);
    if (current) free(current);
    *argc = 0;
    return NULL;

}

void 
csfree(char *s)
{
    if(s == NULL) return;
    free(s - sizeof(struct cstring));
}

void 
csfreesplitres(char **tokens, int count)
{
    if(!tokens) return;
    while(count--)
        csfree(tokens[count]);
    free(tokens);
}

void 
cstolower(char *s)
{
    int len = cslen(s), j;

    for(j = 0; j < len; j++)
        tolower(s[j]);
}