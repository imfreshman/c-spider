#include "spider.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



/* Global vars */
spiderConfig g_conf;

void usage()
{
    fprintf(stderr,"Usage: ./spider [Options]\n");
    fprintf(stderr,"\nOptions:\n");
    fprintf(stderr,"  -h\t: this help\n");
    fprintf(stderr,"  -v\t: print spiderq's version\n");
    fprintf(stderr,"  -d\t: run program as a daemon process\n\n");
    exit(1);
}

int deamon()
{
    int fd;
    pid_t pid;

    umask(0);

    if( (pid = fork()) < 0 )
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "deamon() can't fork. errno %d", errno);
    else if(pid != 0)
        exit(0);
    
    setsid();

    SPIDER_LOG(SPIDER_LEVEL_INFO, "Daemonized...pid=%d",getpid());

    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }

    if(g_conf.logfile != NULL && \
        (fd = open(g_conf->logfile, O_RDWR | O_APPEND | O_CREAT, 0)) != -1)
    {
        dup2(fd, STDOUT_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }

}

int getDownloadAbsolutePath(char *buf)
{
    char cwd[1024];

    if(buf == NULL)
        return -1;

    if(getcwd(cwd, sizeof(cwd)) == NULL)
        return -1;
    
    sprintf(buf, "%s/%s", cwd, DEFAULT_DOWNLOAD_DIR);

    return 0;
}


void 
initApp(int argc, char *argv[])
{
    char configfile[SPIDER_FILE_PATH_MAX];
    char downloadpath[SPIDER_FILE_PATH_MAX];
    int opt;
    int isDeamon = 0;
    
    
    while((opt = getopt(argc, argv, "vhd")) != -1)
    {
        switch(opt)
        {
            case 'v':
                break;
            case 'd':
                isDeamon = 1;
                break;
            case 'h':
            default :
                usage();
                exit(1);
        }
    }

    /* 初始化配置 */
    initConfig(&g_conf);

    /* 获取配置文件路径 */
    memset(configfile, 0x00, SPIDER_FILE_PATH_MAX);
    if(getConfAbsolutePath(configfile) < 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Can't get configfile");
        exit(1);
    }

    /* 读取配置文件 */
    loadConfig(configfile);


    /* 载入模块 */ 

    /* 添加爬虫种子*/

    /* 处理是否以守护进程运行*/
    if( isDeamon ==1 )
        deamon();

    //设定下载路径
    if(getDownloadAbsolutePath(downloadpath) < 0)
    {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Can't get download");
        exit(1);        
    }

    chdir(downloadpath);

}

void 
run()
{

}

int 
main(int argc, char *argv[])
{
    /* 初始化环境 */
    initApp(argc, argv);

    /* 开始主业务流程 */
    run();


    exit(0);
}