#ifndef __SPIDER_H__
#define __SPIDER_H__
#include "log.h"
#include "config.h"
#include "common.h" 

#define DEFAULT_DOWNLOAD_DIR "download"

extern spiderConfig g_conf;


/*
 *  处理命令行参数
 */


/*
 * 守护进程
 */ 
int deamon();

/*
 *初始化环境
 *返回值：成功返回 0  失败返回 -1
 */
void initApp(int, char **);

/*
 *
 */
//int run();

#endif