/*******************************************************************************
* 服务器程序代码server.c
* 2019-11-30 hj实现
*
********************************************************************************/
#include "config.h"

int onlinefd[3];//正在游戏的玩家
int sum=0;//准备的玩家数--控制发牌
int flag=0;//连入的客户端--控制能否再连接客户端
int dizhu;//地主编号
int Cards[55] = { 0 };//记录每张牌是否被用过( 丢弃cards[0]）

/*********************************************
函数名：main
功能：斗地主服务器main函数入口
参数：无
返回值：正常退出返回 0 否则返回 1
**********************************************/
int main(void)
{
	/*声明服务器监听描述符和客户链接描述符*/
	int i , n , ret , maxi , maxfd , listenfd , connfd , sockfd ;

	socklen_t clilen;
	pthread_t pid;
	/*struct timeval time; 用于控制阻塞时间
	time.tv_sec=2;
	time.tv_usec=0;*/

	/*套接字选项*/
	int opt = 1;
 
	/*声明服务器地址和客户地址结构*/
	struct sockaddr_in servaddr , cliaddr;
	
	/*声明消息变量*/
	Message message;
	/*声明描述符集*/
	fd_set rset , allset;
	//nready为当前可用的描述符数量  
	int nready , client_sockfd[3];
	/*声明消息缓冲区*/
	char buf[MAX_LINE];

	pthread_create(&pid , NULL , (void *)fapai , NULL);//控制发牌的线程

	/*(1) 创建套接字*/
	if((listenfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket error.\n");
		exit(1);
	}//if

	/*(2) 初始化地址结构*/
	bzero(&servaddr , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	/*(3) 绑定套接字和端口*/
	setsockopt(listenfd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
	/*SOL_SOCKET 套接字通用选项 SO_REUSEADDR 允许重用本地地址和端口
	有两种套接口的选项：一种是布尔型选项，允许或禁止一种特性；另一种是整形或结构选项。允许一个布尔型选项，则将optval指向非零整形数；禁止一个选 项optval指向一个等于零的整形数。*/
	
	if(bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
	{
		perror("bind error.\n");
		exit(1);
	}//if

	/*(4) 监听*/
	if(listen(listenfd , LISTENEQ) < 0)
	{
		perror("listen error.\n");
		exit(1);
	}//if	

	/*(5) 首先初始化客户端描述符集*/
	maxfd = listenfd;
	maxi = -1;
	for(i=0; i<3; ++i)
	{
		client_sockfd[i] = -1;
		onlinefd[i] = -1;
	}//for

	/*清空allset描述符集*/
	FD_ZERO(&allset);

	/*将监听描述符加到allset中*/
	FD_SET(listenfd , &allset);

	/*(6) 接收客户链接*/
	while(1)
	{
		rset = allset;
		/*得到当前可读的文件描述符数*/
		nready = select(maxfd+1 , &rset , NULL , NULL , 0);

		/*确定一个或多个套接口的状态，如需要则等待。
#include <sys/select.h>
int select( int nfds, fd_set FAR* readfds,　fd_set * writefds, fd_set * exceptfds,　const struct timeval * timeout);
nfds：是一个整数值，是指集合中所有文件描述符的范围，即所有文件描述符的最大值加1，不能错！在Windows中这个参数的值无所谓，可以设置不正确。
readfds：（可选）指针，指向一组等待可读性检查的套接口。
writefds：（可选）指针，指向一组等待可写性检查的套接口。
exceptfds：（可选）指针，指向一组等待错误检查的套接口。
timeout：select()最多等待时间，对阻塞操作则为NULL。*/
		
		/*测试listenfd是否在rset描述符集中*/
		if(FD_ISSET(listenfd , &rset))
		{
			/*接收客户端的请求*/
			clilen = sizeof(cliaddr);
			if((connfd = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
			{
				perror("accept error.\n");
				exit(1);
			}//if
			
			printf("服务器: %s 加入游戏\n", inet_ntoa(cliaddr.sin_addr));
			flag++;

			/*查找空闲位置，设置客户链接描述符*/
			for(i=0; i< FD_SETSIZE; ++i)
			{
				if(client_sockfd[i] < 0)
				{
					client_sockfd[i] = connfd; /*将处理该客户端的链接套接字设置在该位置*/
					break;				
				}//if
			}//for

			if(i == FD_SETSIZE)
			{	
				perror("too many connection.\n");
				exit(1);	
			}//if

			if(flag>=4)//连入的客户端数
			{
				perror("游戏已开始");
				sprintf(buf , "%s", "游戏人数已满\n");
				send(connfd , buf , sizeof(buf) , 0);
			}

			else
			{
				sprintf(buf , "%s", "已进入房间,等待所有玩家准备完毕\n");
				send(connfd , buf , sizeof(buf) , 0);
				/* 将来自客户的连接connfd加入描述符集 */
				FD_SET(connfd , &allset);

				/*新的连接描述符 -- for select*/
				if(connfd > maxfd)
					maxfd = connfd;
				
				/*max index in client_sockfd[]*/
				if(i > maxi)
					maxi = i;
	
				/*判断有没有请求需要处理*/
				if(--nready <= 0)
					continue;
			}
		}//if	
		/*接下来逐个处理连接描述符（请求）*/
		for(i=0 ; i<=maxi ; ++i)
		{
			if((sockfd = client_sockfd[i]) < 0)
				continue;	
			if(FD_ISSET(sockfd , &rset))
			{
				/*如果当前没有可以读的套接字，退出循环*/
				if(--nready < 0)
					break;							
				pthread_create(&pid , NULL , (void *)handleRequest , (void *)&sockfd);
				
				usleep(100000);//睡眠,让创建线程比删除先执行
				/*清除处理完的链接描述符*/
				FD_CLR(sockfd , &allset);
				client_sockfd[i] = -1;
			}//if

		}//for
	}//while
		
	close(listenfd);
	return 0;
}

/*处理客户请求的线程*/
void* handleRequest(int *fd)
{
	int sockfd , ret , n , i;
	/*声明消息变量*/
	Message message;
	/*声明消息缓冲区*/
	char buf[MAX_LINE];

	sockfd = *fd;

	memset(buf , 0 , MAX_LINE);
	memset(&message , 0 , sizeof(message));
	//接收用户发送的消息
	n = recv(sockfd , buf , sizeof(buf)+1 , 0);
	if(n <= 0)
	{
		//关闭当前描述符，并清空描述符数组 
		fflush(stdout);
		close(sockfd);
		*fd = -1;
		printf("来自%s的退出请求！\n", inet_ntoa(message.sendAddr.sin_addr));		
		return NULL;			
	}//if				
	else{
		memcpy(&message , buf , sizeof(buf));								
		switch(message.msgType)
		{
		case 1:						
			{
				printf("%s 已经准备！\n", inet_ntoa(message.sendAddr.sin_addr));
				/*查找空闲位置，放置在线的链接套接字*/
				for(i=0; i<3; ++i)
				{
					if(onlinefd[i] < 0)
					{
						onlinefd[i] = sockfd; 
						sum++;//准备的玩家数加一
						break;				
					}//if
				}//for
				enterGame(&onlinefd[i]);
				break;
			}//case	
		case 2:						
			{
				printf("%s 已经退出！\n", inet_ntoa(message.sendAddr.sin_addr));
				flag--;
				break;
			}//case	
		default:
			printf("未知操作\n");
			break;
		}//switch					
	}//else				

}

//控制发牌的线程
void* fapai()
{
	int i;
	char buf[MAX_LINE];
	while(1)
	{
		if(sum==3)
		{
			for(i=0; i<3; ++i)
			{
				sprintf(buf , "%s", "游戏开始\n");
				send(onlinefd[i] , buf , sizeof(buf) , 0);
			}
			dizhu = rand() % 3;//随机产生一个地主
			sendCards();//发牌
			for(i=1; i<55; ++i)//初始化标志数组
			{
				Cards[i] = 0;
			}
			sum=0;
		}
	}
}

void sendCards()
{
    int i, j, iCount = 0;
    char buf[MAX_LINE];
    Message message;
    int Player_A[18]={0};
    int Player_B[18]={0};
    int Player_C[18]={0};
    int Ground[4]={0};//Abandon all [0]

    for (i = 1; i <= 3; i++)//获取底牌
    {
        Ground[i] = getOneCard();
    }
    sortCards(Ground, 4);//按牌大小排序
    for (i = 1; i <= 17; i++)//为三个玩家分别发牌
    {
        Player_A[i] = getOneCard();
        Player_B[i] = getOneCard();
        Player_C[i] = getOneCard();
    }
    puts("玩家的手牌:");
        sortCards(Player_A, 18);
        sortCards(Player_B, 18);
        sortCards(Player_C, 18);
    puts("玩家0:");//打印A的牌
        showCards(Player_A, 17);
	memset(&message , 0 , sizeof(message));
	memset(buf , 0 , MAX_LINE);
	
	message.msgType = 4;//发送A的手牌
	message.turn=0;//发送A的序号
	memcpy(message.card , Player_A , sizeof(Player_A));
	memcpy(buf , &message , sizeof(message));	
	send(onlinefd[0] , buf , sizeof(buf) , 0);
    puts("\n\n玩家1:");//打印B的牌
        showCards(Player_B, 17);
	memset(&message , 0 , sizeof(message));
	memset(buf , 0 , MAX_LINE);
	
	message.msgType = 4;//发送B的手牌
	message.turn=1;//发送B的序号
	memcpy(message.card , Player_B , sizeof(Player_B));
	memcpy(buf , &message , sizeof(message));	
	send(onlinefd[1] , buf , sizeof(buf) , 0);
    puts("\n\n玩家2:");//打印C的牌
        showCards(Player_C, 17);
	memset(&message , 0 , sizeof(message));
	memset(buf , 0 , MAX_LINE);
	
	message.msgType = 4;//发送C的手牌
	message.turn=2;//发送C的序号
	memcpy(message.card , Player_C , sizeof(Player_C));
	memcpy(buf , &message , sizeof(message));	
	send(onlinefd[2] , buf , sizeof(buf) , 0);
    printf("\n\n地主为玩家%d 底牌为:\n",dizhu);
        showCards(Ground, 3);//打印底牌
	memset(&message , 0 , sizeof(message));
	memset(buf , 0 , MAX_LINE);
	
	message.msgType = 5;//给农民发送底牌
	memcpy(message.card , Ground , sizeof(Ground));
	memcpy(buf , &message , sizeof(message));	
	send(onlinefd[(dizhu+1)%3] , buf , sizeof(buf) , 0);
	send(onlinefd[(dizhu+2)%3] , buf , sizeof(buf) , 0);
	
	message.msgType = 6;//给地主发送底牌
	memcpy(buf , &message , sizeof(message));
	send(onlinefd[dizhu] , buf , sizeof(buf) , 0);
	puts(" ");

	memset(&message , 0 , sizeof(message));
	memset(buf , 0 , MAX_LINE);
	
	message.msgType = 7;
	message.turn=dizhu;//发送出牌序号
	memcpy(message.card , Ground , sizeof(Ground));
	memcpy(buf , &message , sizeof(message));
	//统一三个客户端的出牌序号	
	send(onlinefd[(dizhu+1)%3] , buf , sizeof(buf) , 0);
	send(onlinefd[(dizhu+2)%3] , buf , sizeof(buf) , 0);
	send(onlinefd[dizhu] , buf , sizeof(buf) , 0);
}
int getOneCard()//通过随机数取得一张没用过的牌
{
    int iRandNum;
    srand(time(0));//取时间随机数（种子）
    iRandNum = rand() % 54 + 1; //rand()%m,产生0- m-1的随机数
    while (Cards[iRandNum] == 1)//被用过的牌会重新取随机数
    {
        iRandNum = rand() % 54 + 1;
    }
    Cards[iRandNum] = 1;
    return iRandNum;
}
void showCards(int *Player, int Count)//打印玩家的牌
{
    int i;
    for (i = 1; i <= Count; i++)
    {
        if (Player[i] == 53)
        {
            printf(" 小王");
        }
        else if (Player[i] == 54)
        {
            printf(" 大王");
        }
        else if (Player[i] % 4 == 1)
        {
            printf(" 黑");
            changeCard(Player[i]);
        }
        else if (Player[i] % 4 == 2)
        {
            printf(" 红");
            changeCard(Player[i]);
        }
        else if (Player[i] % 4 == 3)
        {
            printf(" 方");
            changeCard(Player[i]);
        }
        else
        {
            printf(" 梅");
            changeCard(Player[i]);
        }
    }
}
void changeCard(int Card)//将数字和纸牌进行对换
{
    int temp = Card/4.0000001;//用于控制0和13的越界
    switch (temp)
    {
    case 0:printf("3 "); break;
    case 1:printf("4 "); break;
    case 2:printf("5 "); break;
    case 3:printf("6 "); break;
    case 4:printf("7 "); break;
    case 5:printf("8 "); break;
    case 6:printf("9 "); break;;
    case 7:printf("10 "); break;
    case 8:printf("J "); break;
    case 9:printf("Q "); break;
    case 10:printf("K "); break;
    case 11:printf("A "); break;
    case 12:printf("2 "); break;
    }
}
void sortCards(int *Nums, int Length)//排序手中的牌
{
    int i, j, iTemp;
    for (i = 1; i <= Length - 1; i++)
    {
        for (j = 1; j <= Length - 2; j++)
        {
            if (Nums[j]>Nums[j + 1])
            {
                iTemp = Nums[j];
                Nums[j] = Nums[j + 1];
                Nums[j + 1] = iTemp;
            }
        }
    }
}
