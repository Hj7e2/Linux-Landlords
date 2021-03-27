/*******************************************************************************
* 基本配置文件 -- 包含所需头文件
* 用户信息结构体定义
********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h> /*使用memcpy所需的头文件*/

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

/*FD_SETSIZE定义描述符集的大小，定义在sys/types.h中*/
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

#define MAX_LINE  8192
#define PORT  8888
#define LISTENEQ  6000

/*定义服务器 -- 客户端 消息传送结构体*/
typedef struct _Message{
	int msgType;	/*消息类型 */
	int card[18];   /*卡牌*/
	struct sockaddr_in sendAddr; /*发送者IP*/
	int turn;	/*出牌序号*/
}Message;

/*定义在线用户*/
extern int onlinefd[3];
extern int flag;
extern int dizhu;

//函数声明
void enterGame(int *sockfd);

void* handleRequest(int *fd);//处理请求的线程

void* fapai();//控制发牌的线程

int getOneCard();//通过随机数取得一张没用过的牌

void showCards(int *Player, int Count);//打印玩家的牌

void changeCard(int Card);//将数字和纸牌进行对换

void sortCards(int *Nums, int Length);//排序手中的牌

void sendCards();//发牌函数

