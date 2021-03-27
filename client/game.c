/*******************************************************************************
* 客户端用户游戏界面处理实现文件
* 2019-11-30 hj实现
*
********************************************************************************/

#include "config.h"

int owncard[21];//手牌
int lastcard[21];//上一次出牌
int lastlength=0;//上一次出牌的张数
int length=2;//手牌数，2无意义，大于等于2皆可，后面会覆盖
int pass=0;//跳过计数
int turn;//出牌顺序
int player;//玩家编号
int flag;
/***********************************************
函数名：recvMsg
功能：接收服务器发来的消息
参数：sockfd -- 套接字描述符
返回值：无返回值
*************************************************/
void recvMsg(int *sockfd)
{
	int connfd = *sockfd;
	int nRead;

	char buf[MAX_LINE];
	Message message;

	while(1)
	{

		memset(buf , 0 , MAX_LINE);
		memset(&message , 0 , sizeof(message));
		//接收服务器发来的消息
		nRead = recv(connfd , buf , sizeof(buf) , 0);
		//recv函数返回值 <0 出错  =0 链接关闭  >0接收到的字节数
		if(nRead <= 0)
		{
			printf("您已经异常掉线，请重新登录！\n");
			close(connfd);
			exit(0);
		}//if

		else
		{
			memcpy(&message , buf , sizeof(buf));							
			switch(message.msgType)
			{
			case 1:	
				pass=0;
				memcpy(lastcard , message.card , sizeof(lastcard));//把玩家出的牌保存到数组中
				lastlength=message.turn;

				printf("玩家%d出牌:\n",turn);
				showCards(message.card,message.turn);
				printf("\n");
				printf("-------------------------------------\n");
				turn=(turn+1)%3;
				printf("下面是玩家%d出牌:\n",turn);
				printf("-------------------------------------\n");
				break;
			case 2:	
				pass++;
				if(pass==2)
				{
					memset(lastcard , 0 , sizeof(lastcard));
					lastlength=0;
					pass=0;
				}

				printf("玩家%d跳过出牌\n", turn);
				printf("-------------------------------------\n");
				turn=(turn+1)%3;
				printf("下面是玩家%d出牌:\n",turn);
				printf("-------------------------------------\n");
				break;
			case 3:	
				printf("%s", "有用户退出，游戏结束\n");
				printf("-------------------------------------\n");
				close(connfd);
				exit(0);
				break;
			case 4:	
				player=message.turn;
				printf("玩家%d: 您的手牌为:\n",player);
				showCards(message.card, 17);
				memcpy(owncard , message.card , sizeof(message.card));
				printf("\n");
				printf("************************************************************************************************\n");
				break;
			case 5:	
				printf("底牌为:\n");
				showCards(message.card, 3);
				printf("\n");
				printf("-------------------------------------\n");
				break;
			case 6:	
				printf("您已成为地主,底牌为:\n");
				showCards(message.card, 3);
				printf("\n");
				printf("-------------------------------------\n");
				owncard[18]=message.card[1];
				owncard[19]=message.card[2];
				owncard[20]=message.card[3];
				sortCards(owncard,21);
				printf("玩家%d: 您现在的手牌为:\n",player);
				showCards(owncard,20);
				printf("\n");
				printf("************************************************************************************************\n");
				break;
			case 7:	
				turn=message.turn;//统一客户端出牌顺序
				if(player == turn)//设置每个玩家的手牌数
					length=21;
				else
					length=18;
				printf("地主出牌\n");
				printf("-------------------------------------\n");	
				break;
			case 8:
				system("reset");
				printf("用户%d获胜:\n",message.turn);
				close(connfd);
				exit(0);
				break;
			default:
				break;
			}
		}

	}//while	
}

/***********************************************
函数名：enterGame
功能：用户准备后进入游戏
参数：sockfd -- 套接字描述符
返回值：无返回值
*************************************************/
void enterGame(int *fd)
{
	int choice,ret,i,j,x;
	int temp[21];//存放要发送的牌
	int del[21];//存放要删除牌的下标
	char buf[MAX_LINE];
	Message message;	/*消息对象*/
	pthread_t pid;	/*处理接收消息线程*/
	int sockfd=*fd;	
	/*进入游戏主界面*/
	gameInterface();

	usleep(100000);//睡眠，以便界面先出现
	/*创建接收消息线程*/
	ret = pthread_create(&pid , NULL , (void *)recvMsg , (void *)&sockfd);
	if(ret != 0)
	{
		printf("软件异常，请重新登录！\n");
		memset(&message , 0 , sizeof(message));
		memset(buf , 0 , MAX_LINE);

		message.msgType = 3;
		memcpy(buf , &message , sizeof(buf));
		send(sockfd , buf , sizeof(buf) , 0);
		close(sockfd);
		exit(1);
	}

	/*进入处理用户发送消息缓冲区*/
	while(1)
	{
		if(length-1<=0)
		{
			system("reset");
			printf("用户%d获胜:\n",message.turn);

			message.msgType = 4;
			message.turn=player;
			memcpy(buf , &message , sizeof(message));
			send(sockfd , buf , sizeof(buf) , 0);
			close(sockfd);
			exit(0);
		}

		memset(&message , 0 , sizeof(message));
		memset(buf , 0 , MAX_LINE);
		memset(temp , 0 , 21);

		flag=1;
		while(flag)
		{
			setbuf(stdin,NULL);
			scanf("%d",&choice);
			while(choice != 1 && choice != 2 && choice != 3)
			{
				printf("未知操作，请重新输入！\n");
				setbuf(stdin,NULL);
				scanf("%d",&choice);
			}//while

			switch(choice)
			{
			case 1: /*出牌*/
				if(player != turn)
				{
					printf("还未轮到您的回合\n");
					printf("-------------------------------------\n");
					break;
				}
				else
				{
					i=1;//数组都从1开始
					message.msgType = 1;
					memset(temp , 0 , sizeof(temp));
					memset(del , 0 , sizeof(del));
					printf("请输入要出的牌的序号,输入0出牌\n");
					setbuf(stdin , NULL);
					scanf("%d",&x);
					while(x!=0)
					{
						if(i>=length)
							printf("已无多余手牌，请直接出牌\n");
						if(x>=length||x<=0)
						{
							printf("手牌中没有此牌,请重新输入\n");
							setbuf(stdin , NULL);
							scanf("%d",&x);
						}
						else
						{	
							del[i]=x;//要删除牌的下标
							temp[i++]=owncard[x];
							setbuf(stdin , NULL);
							scanf("%d",&x);
						}
					}						
					
					sortCards(temp, i);//对要出的牌进行排序
					if(canSend(temp,lastcard,i-1,lastlength))
					{
						message.turn = i-1;//要发送的牌的数目
						memcpy(message.card , temp , sizeof(temp));
						memcpy(buf , &message , sizeof(message));
						send(sockfd , buf , sizeof(buf) , 0);
						for(j=1;j<i;j++)//删除出过的牌
						{	
							del[j]=del[j]-j+1;//删除一张，下标减一
							delCard(owncard,del[j],length--);				
						}

						system("reset");

						sortCards(owncard, length);//对剩下的牌进行排序
					printf("************************************************************************************************\n");
						printf("玩家%d:您现在的手牌为:\n",player);
						showCards(owncard, length-1);
						printf("\n");
					printf("************************************************************************************************\n");
						turn=(turn+1)%3;
						printf("下面是玩家%d出牌:\n",turn);
						printf("-------------------------------------\n");

						flag=0;
					}
					else
					{
						printf("您输入的手牌比上家小，或不符合牌型，请重新输入\n");
					}
					
					break;
				}
			case 2: /*跳过*/
				if(player != turn)
				{
					printf("还未轮到您的回合\n");
					printf("-------------------------------------\n");
					break;
				}
				else if(lastlength==0)
				{
					printf("您不可以跳过\n");
					break;
				}
				else
				{
					flag=0;
					message.msgType = 2;
					message.turn=player;
					memcpy(buf , &message , sizeof(message));
					send(sockfd , buf , sizeof(buf) , 0);
					turn=(turn+1)%3;
					system("reset");
					printf("************************************************************************************************\n");
					printf("玩家%d:您现在的手牌为:\n",player);
					showCards(owncard, length-1);
					printf("\n");
					printf("************************************************************************************************\n");
					printf("下面是玩家%d出牌:\n",turn);
					printf("-------------------------------------\n");
					break;
				}
			case 3: /*退出*/
				flag=0;
				message.msgType = 3;
				message.turn=player;
				memcpy(buf , &message , sizeof(message));
				send(sockfd , buf , sizeof(buf) , 0);
				close(sockfd);
				exit(0);

				break;
			default: 	/*未知操作类型*/
				break;
			}//switch
		}//while(flag)
	}//while
}

void showCards(int *Player, int Count)//打印玩家的牌
{
    int i;
    printf(" ");
    for (i = 1; i <= Count; i++)
    {
	if(i<10)
		printf("%d    ",i);
	else
		printf("%d   ",i);
    }
    printf("\n");
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
    int temp = Card/4.0000001;
    switch (temp)
    {
    case 0:printf("3 "); break;
    case 1:printf("4 "); break;
    case 2:printf("5 "); break;
    case 3:printf("6 "); break;
    case 4:printf("7 "); break;
    case 5:printf("8 "); break;
    case 6:printf("9 "); break;;
    case 7:printf("10"); break;
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

void delCard(int cards[],int number,int length)//删除对应下标的牌
{
	int i;
	for(i=number;i<length-1;i++)
	{
		cards[i]=cards[i+1];
	}
	cards[length-1]=0;
}

int canSend(int card[] ,int lastcard[] ,int length ,int lastlength)
{
	Cardlevel a,b;
	a=whatlevel(card,length);
	b=whatlevel(lastcard,lastlength);
	if(lastlength==0)//如果其他玩家都跳过，判断是否符合牌型
	{
		if(a.kind!=-1&&a.kind!=0)
			return 1;
		else
			return 0;
	}
	else if(length==lastlength)//判断牌数是否相等
	{
		if(a.kind==b.kind)//判断牌型是否一致
		{
			if(a.level>b.level)
				return 1;
			else
				return 0;
		}
		else if(a.kind=11||a.kind==14)//如果出的是炸弹
			return 1;
		else
			return 0;
	}
	else if(a.kind==11||a.kind==14)//如果出的是炸弹
		return 1;
	else
		return 0;
}

Cardlevel whatlevel(int sendcard[], int length)
{
	int i,j;
	int card[21];
	/*kind 对应的牌型 kind为-1则不符合牌型
	char cardtype[15][]={"弃权0","单张1","对子2","三张3","三带一4","三带二5","顺子6","连对7",
				"飞机不带8","飞机带单9","飞机带对10","炸弹11","四带二12","四带两对13","王炸14"};*/
	Cardlevel a;
	for(i=1;i<=length;i++)
		card[i]=sendcard[i]/4.00000001;

	switch(length)
	{
	case 0:
		a.kind=0;
		break;
	case 1://如果是单张
		a.kind=1;
		a.level=card[1];
		break;
	case 2:
		if(card[1]==card[2]&&card[1]!=13)//对子
		{
			a.kind=2;
			a.level=card[1];
		}
		else if(card[1]==13&&card[2]==13)//王炸
			a.kind=14;
		else
			a.kind=-1;
		break;
	case 3:
		if(card[1]==card[2]&&card[1]==card[3])//三张
		{
			a.kind=3;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 4:
		if(card[1]==card[2]&&card[1]==card[3])//三带一
		{
			a.kind=4;
			a.level=card[1];
		}
		else if(card[2]==card[3]&&card[2]==card[4])//三带一
		{
			a.kind=4;
			a.level=card[2];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[1]==card[4])//炸弹
		{
			a.kind=11;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 5:
		if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5])//三带二
		{
			a.kind=5;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[3]==card[4]&&card[3]==card[5])//三带二
		{
			a.kind=5;
			a.level=card[3];
		}
		else if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 6:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[3]==card[4]&&card[5]==card[6]&&card[1]==card[3]-1&&card[1]==card[5]-2)//连对
		{
			a.kind=7;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[1]==card[4]-1)//飞机不带
		{
			a.kind=8;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[1]==card[4])//四带二
		{
			a.kind=12;
			a.level=card[1];
		}
		else if(card[3]==card[4]&&card[3]==card[5]&&card[3]==card[6])//四带二
		{
			a.kind=12;
			a.level=card[3];
		}
		else
			a.kind=-1;
		break;
	case 7:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5&&card[1]==card[7]-6)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 8:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5
		&&card[1]==card[7]-6&&card[1]==card[8]-7)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[3]==card[4]&&card[5]==card[6]&&card[7]==card[8]
			&&card[1]==card[3]-1&&card[1]==card[5]-2&&card[1]==card[7]-3)//连对
		{
			a.kind=7;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[1]==card[4]-1)//飞机带单
		{
			a.kind=9;
			a.level=card[1];
		}
		else if(card[3]==card[4]&&card[3]==card[5]&&card[6]==card[7]&&card[6]==card[8]&&card[3]==card[6]-1)//飞机带单
		{
			a.kind=9;
			a.level=card[3];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[1]==card[4]&&card[5]==card[6]&&card[7]==card[8])//四带两对
		{
			a.kind=13;
			a.level=card[1];
		}
		else if(card[3]==card[4]&&card[3]==card[5]&&card[3]==card[6]&&card[1]==card[2]&&card[7]==card[8])//四带两对
		{
			a.kind=13;
			a.level=card[3];
		}
		else if(card[5]==card[6]&&card[5]==card[7]&&card[5]==card[8]&&card[1]==card[2]&&card[3]==card[4])//四带两对
		{
			a.kind=13;
			a.level=card[5];
		}
		else
			a.kind=-1;
		break;
	case 9:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5
		&&card[1]==card[7]-6&&card[1]==card[8]-7&&card[1]==card[9]-8)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[7]==card[8]&&card[7]==card[9]
			&&card[1]==card[4]-1&&card[1]==card[7]-2)//飞机不带
		{
			a.kind=8;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 10:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5
		&&card[1]==card[7]-6&&card[1]==card[8]-7&&card[1]==card[9]-8&&card[1]==card[10]-9)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[3]==card[4]&&card[5]==card[6]&&card[7]==card[8]&&card[9]==card[10]
			&&card[1]==card[3]-1&&card[1]==card[5]-2&&card[1]==card[7]-3&&card[1]==card[9]-4)//连对
		{
			a.kind=7;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[1]==card[4]-1
		&&card[7]==card[8]&&card[9]==card[10])//三带二
		{
			a.kind=5;
			a.level=card[1];
		}

		else if(card[5]==card[6]&&card[5]==card[7]&&card[8]==card[9]&&card[8]==card[10]&&card[6]==card[9]-1
		&&card[1]==card[2]&&card[3]==card[4])//三带二
		{
			a.kind=5;
			a.level=card[5];
		}
		else
			a.kind=-1;
		break;
	case 11:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5
		&&card[1]==card[7]-6&&card[1]==card[8]-7&&card[1]==card[9]-8&&card[1]==card[10]-9&&card[1]==card[11]-10)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	case 12:
		if(card[1]==card[2]-1&&card[1]==card[3]-2&&card[1]==card[4]-3&&card[1]==card[5]-4&&card[1]==card[6]-5&&card[1]==card[7]-6
		&&card[1]==card[8]-7&&card[1]==card[9]-8&&card[1]==card[10]-9&&card[1]==card[11]-10&&card[1]==card[12]-11)//顺子
		{
			a.kind=6;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[3]==card[4]&&card[5]==card[6]&&card[7]==card[8]&&card[9]==card[10]&&card[11]==card[12]
			&&card[1]==card[3]-1&&card[1]==card[5]-2&&card[1]==card[7]-3&&card[1]==card[9]-4&&card[1]==card[11]-5)//连对
		{
			a.kind=7;
			a.level=card[1];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[7]==card[8]&&card[7]==card[9]
		&&card[1]==card[4]-1&&card[1]==card[7]-2)//飞机带单
		{
			a.kind=9;
			a.level=card[1];
		}
		else if(card[4]==card[5]&&card[4]==card[6]&&card[7]==card[8]&&card[7]==card[9]&&card[10]==card[11]&&card[10]==card[12]
		&&card[4]==card[7]-1&&card[4]==card[10]-2)//飞机带单
		{
			a.kind=9;
			a.level=card[4];
		}
		else if(card[1]==card[2]&&card[1]==card[3]&&card[4]==card[5]&&card[4]==card[6]&&card[7]==card[8]&&card[7]==card[9]
			&&card[10]==card[11]&&card[10]==card[12]&&card[1]==card[4]-1&&card[1]==card[7]-2&&card[1]==card[10]-3)//飞机不带
		{
			a.kind=8;
			a.level=card[1];
		}
		else
			a.kind=-1;
		break;
	}
	return a;
}
