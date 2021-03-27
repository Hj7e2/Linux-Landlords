/*******************************************************************************
* 客户端用户游戏界面处理实现文件
* 2019-11-30 hj实现
*
********************************************************************************/

#include "config.h"

/***********************************************
函数名：enterGame
功能：用户准备后进入游戏
参数：sockfd -- 套接字描述符
返回值：正常退出返回 0 ， 否则返回 1
*************************************************/
void enterGame(int *fd)
{
	int n,sockfd,i;
	/*消息发送缓冲区*/
	char buf[MAX_LINE];
	/*消息内容*/
	Message message;

	memset(buf , 0 , MAX_LINE);	
	memset(&message , 0 , sizeof(message));
	
	sockfd = *fd;

	while(1)
	{
		//接收用户发送的消息
		n = recv(sockfd , buf , sizeof(buf)+1 , 0);
		if(n <= 0)
		{
			//关闭当前描述符
			close(sockfd);
			return ;					
		}//if				
		else{		
			memcpy(&message , buf , sizeof(buf));				
			switch(message.msgType)
			{
			case 1:
				{
					message.msgType = 1;//发送用户出牌信息
					memcpy(buf , &message , sizeof(message));
					for(i=0; i<3; ++i)//发送给除出牌者外的人
						if(onlinefd[i] != sockfd)
							send(onlinefd[i] , buf , sizeof(buf) , 0);
					printf("玩家%d出牌:\n",dizhu++%3);
					showCards(message.card,message.turn);
					printf("\n");	
				}
				break;

			case 2:
				{
					message.msgType = 2;//发送用户跳过信息
					memcpy(buf , &message , sizeof(message));
					for(i=0; i<3; ++i)//发送给除跳过者外的人
						if(onlinefd[i] != sockfd)
							send(onlinefd[i] , buf , sizeof(buf) , 0);
					printf("玩家%d跳过出牌\n",dizhu++%3);
				}
				break;		
		
			case 3:
				{
					printf("用户%d退出！\n", message.turn);
					for(i=0; i<3; ++i)
					{
						if(onlinefd[i] != sockfd)
						{
							memset(&message , 0 , sizeof(message));
							memset(buf , 0 , MAX_LINE);
	
							message.msgType = 3;//发送用户退出信息
							memcpy(buf , &message , sizeof(message));	
							send(onlinefd[i] , buf , sizeof(buf) , 0);
						}
						onlinefd[i] = -1; /*将用户从房间中删除*/
					}//for
					flag=0;//断开所有客户端连接
					close(sockfd);
					break;
				}
			case 4:
				{
					/*游戏结束*/
					printf("用户%d胜出！\n", message.turn);
					for(i=0; i<3; ++i)
					{
						if(onlinefd[i] != sockfd)
						{
							message.msgType = 8;//发送用户胜利信息
							memcpy(buf , &message , sizeof(message));	
							send(onlinefd[i] , buf , sizeof(buf) , 0);
						}
						onlinefd[i] = -1; /*将用户从房间中删除*/
					}//for
					flag=0;
					close(sockfd);
					break;
				}
			default:			
				break;
			}//switch
		}//else
	}//while	
}

