#include "config.h"
#include "cstring.h"
#include "common.h"
#include "spider.h"
#include "log.h"
#include <strings.h>
#include <stdio.h>
#include <unistd.h>


void 
init_params_queue(params_queue *p)
{
    p->bufSize = 0;
    INIT_LIST_HEAD(&p->head);
}

void 
push_params_queue(params_queue *p, struct list_head *e)
{
    if(p == NULL || e == NULL)
        return;
    
    list_add_tail(e, &p->head);
    p->bufSize++;

}

void 
pop_params_queue(params_queue *p, struct list_head **e)
{
    if(p == NULL || *e == NULL)
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


/*
 * config file parsing
 */

int 
yesnotoi(char *s)
{
    if( !strcasecmp(s, "yes" )) return 1;
    else if ( !strcasecmp(s,"NO"))  return 0;
    else return -1;
}

void 
initConfig(spiderConfig *conf)
{
    conf->max_job_num =10;
    conf->seeds = NULL;
    conf->include_prefixes = NULL;
    conf->exclude_prefixes = NULL;
    conf->logfile = NULL;
    conf->log_level = 0;
    conf->max_depth = INF;
    conf->make_hostdir = 0;
    conf->module_path = NULL;
    conf->stat_interval = 0;

    init_params_queue(&conf->modules);
    init_params_queue(&conf->accept_types);
}

void
loadConfigFromString(char *conf)
{
    char *err = NULL;
    int linenum = 0, totlines, i;
    int slaveof_linenum =0;
    char **lines;
    
    lines = cssplitlen(conf, strlen(conf), "\n", 1, &totlines);  
    for(i = 0; i < totlines; i++ )
    {
        char **argv;
        int argc;

        linenum = i + 1;

        //移除字符串前置空白和后缀空白
        lines[i] = csstrim(lines[i], "\t\r\n");

        //跳过空白行
        if(lines[i][0] == '#' || lines[i][0] == '\0') continue;

        SPIDER_LOG(SPIDER_LEVEL_DEBUG, "lines:[%s]", lines[i]);
        //将字符串分割成多个参数
        argv = cssplitargs(lines[i], &argc);
        if(argv == NULL)
        {
            err = "Unbalanced quotes in configuration line";
            goto loaderr; 
        }

        /*for(int j = 0; j < argc; j++)
            SPIDER_LOG(SPIDER_LEVEL_DEBUG, "argc:[%d] argv:[%s]", j, argv[j]);*/

        /* 跳过空白参数 */
        if(argc == 0)
        {
            csfreesplitres(argv, argc);
            continue;
        }

        //将选项名称转换成小写
        cstolower(argv[0]);

        if(argc == 2)
        {
            if (strcasecmp(argv[0], "max_job_num") == 0) {
                g_conf.max_job_num = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "logfile") == 0) {
                g_conf.logfile = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "include_prefixes") == 0) {
                g_conf.include_prefixes = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "exclude_prefixes") == 0) {
                g_conf.exclude_prefixes = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "seeds") == 0) {
                g_conf.seeds = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "module_path") == 0) {
                g_conf.module_path = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "load_module") == 0) {
                params *p = NULL;
                p = malloc(sizeof(struct params));
                p->elem = strdup(argv[1]);
                push_params_queue(&g_conf.modules, &p->head);
            } else if (strcasecmp(argv[0], "log_level") == 0) {
                g_conf.log_level = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "max_depth") == 0) {
                g_conf.max_depth = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "stat_interval") == 0) {
                g_conf.stat_interval = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "make_hostdir") == 0) {
                g_conf.make_hostdir = yesnotoi(argv[1]);
            } else if (strcasecmp(argv[0], "accept_types") == 0) {
                params *p = NULL;
                p = malloc(sizeof(struct params));
                p->elem = strdup(argv[1]);
                push_params_queue(&g_conf.accept_types, &p->head);             
            } else {
                err = "Unknown directive"; 
                goto loaderr;
            }
        }
        else
        {
            err = "directive must be 'key=value'"; 
            goto loaderr;
        }

        free(argv);
        argv = NULL;
    }
    csfreesplitres(lines,totlines);

    return ;

loaderr:
    SPIDER_LOG(SPIDER_LEVEL_ERROR, "Bad directive in [line:%d] %s", linenum, err);	
}


void 
loadConfig(const char *filename)
{
    char *conf = csempty();
    char buf[SPIDER_CONFIGLINE_MAX + 1];
    FILE *fp;

    /*load the file content */
    if(filename)
    {
        if(filename[0] == '-' && filename[1] == '\0')
            fp = stdin;
        else
        {
            if((fp = fopen(filename, "r")) == NULL)
            {
                SPIDER_LOG(SPIDER_LEVEL_ERROR, "Fatal error, can't open config file '%s'", filename);
                exit(1);
            }
        }
        while(fgets(buf, SPIDER_CONFIGLINE_MAX + 1, fp) != NULL)
            conf = cscat(conf, buf);
        if(fp != stdin) 
            fclose(fp);       
    }

    loadConfigFromString(conf);
   
    
    csfree(conf);
}


int getConfAbsolutePath(char *buf)
{
    char cwd[1024];

    if(buf == NULL)
        return -1;

    if(getcwd(cwd, sizeof(cwd)) == NULL)
        return -1;

    //SPIDER_LOG(SPIDER_LEVEL_DEBUG, "%s",cwd);
    sprintf(buf, "%s/%s", cwd, DEFAULT_CONFIG_FILE);
    return 0;
}