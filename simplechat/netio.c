//本文件包含与网络部分密切相关的变量和函数，如消息收发、客户端管理等
#include "simplechat.h" //部分量在此定义
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
#include <sys/socket.h>
extern socklen_t addrlen;
extern int is_server;
int ready=0;
struct client {
	int s;
	char ip[INET6_ADDRSTRLEN];
	unsigned short port;
	struct client *next;
} *c=NULL; //客户端链表
mtx_t mtx; //互斥锁
char* const commands[]={"/tell","/list","/ping"};
struct client* lookup_client(const int s) //链表查找，返回对应节点
{
	for(struct client *i=c;i!=NULL;i=i->next)
		if(i->s==s)
			return i;
	return NULL;
}
int lookup_user(const int s) //包装函数，供其他功能使用
{
	struct client *i=lookup_client(s);
	return i==NULL?-1:i->s;
}
unsigned int current_users(const int s) //列出用户，若为socket为s的客户端请求则发送给它
{
	unsigned int count=0;
	char *head="SOCKET  PORT IP\n";
	char msg[14+INET6_ADDRSTRLEN];
	s>=0?send_chat(s,head):fputs(head,stdout);
	for(struct client *i=c;i!=NULL;i=i->next)
	{
		count++;
		sprintf(msg,"%c%5d %5u %s\n",s==i->s?'*':' ',i->s,i->port,i->ip);
		s>=0?send_chat(s,msg):fputs(msg,stdout);
	}
	sprintf(msg,"Total: %u\n",count);
	s>=0?send_chat(s,msg):puts(msg);
	return count;
}
int disconnect_client(const int s) //断开值为s的socket
{
	struct client *i,*t;
	int state=0; //控制返回值
	if(close(s)<0) //断开连接
	{
		logmsg(2,"Socket %d close failed",s);
		perror("\033[1;33mSocket close error\033[0m");
		state+=1;
	}
	state+=2;
	//尝试删除链接相关项
	if(is_server&&c!=NULL)
	{
		if(mtx_lock(&mtx)==thrd_error)
			logmsg(2,"mtx_lock failed");
		i=c;
		if(i->s==s)
		{
			c=i->next;
			free(i);
			state-=2;
		}
		else
			while(i->next!=NULL)
			{
				if(i->next->s==s)
				{
					t=i->next;
					i->next=t->next;
					free(t);
					state-=2;
					break;
				}
				else
					i=i->next;
			}
		mtx_unlock(&mtx);
	}
	if(state<2)
		logmsg(1,"Removed client %d",s);
	return state;
}
int accept_client(void *p) //服务器模式下，在子线程中运行，循环接受某个socket的客户端连接
{
	const int sock=*(int *)p;
	int s;
	thrd_t th;
	struct client *t,*i;
	union sockaddrs addr;
	while(is_server)
	{
		if((s=accept(sock,&addr.sa,&addrlen))>0)
		{
			//新建链表项
			if((t=(struct client *)malloc(sizeof(struct client)))==NULL)
			{
				logmsg(3,"Failed to allocate memory");
				fputs("\033[1;33mTrying to record a new client, but no free memory left\033[0m\n",stderr);
				disconnect_client(s);
				continue;
			}
			t->s=s,t->next=NULL;
			//获取IP和端口
			if(inet_ntop(addr.sa.sa_family,addr.sa.sa_family==AF_INET6?&addr.s6.sin6_addr:&addr.s4.sin_addr,t->ip,sizeof(t->ip))==NULL)
				logmsg(2,"Invalid remote IP address");
			t->port=ntohs(addr.s4.sin_port);
			logmsg(1,"Remote address: [%s]:%u",t->ip,t->port);
			//向链表添加数据
			if(mtx_lock(&mtx)==thrd_error) //锁上互斥锁
				logmsg(2,"mtx_lock failed");
			if(c==NULL)
				c=t;
			else
			{
				for(i=c;i->next!=NULL;i=i->next);
				i->next=t;
			}
			mtx_unlock(&mtx); //解锁互斥锁
			if(thrd_create(&th,recv_chat,&(t->s))!=thrd_success&&thrd_detach(th)) //创建接受数据的线程
			{
				logmsg(3,"Failed to create thread");
				fputs("\033[1;33mChat server is not stable due to thread error\033[0m\n",stderr);
			}
			logmsg(1,"Added a new client %d",t->s);
		}
		else
		{
			logmsg(2,"Failed to accept socket");
			perror("\033[1;33mSocket accept error\033[0m");
		}
	}
	return 0;
}
int send_chat(const int s,const char* msg) //向值为s的socket发送字符串msg
{
	int state=0;
	if(s>=0)
	{
		if(send(s,msg,strlen(msg),MSG_NOSIGNAL)<0)
		{
			logmsg(2,"Message send to %d error",s);
			perror("\033[1;33mSocket send error\033[0m");
			return -1;
		}
		is_server?logmsg(0,"A message has been sent to %d",s):logmsg(0,"A message has been sent to server");
	}
	else if(is_server)
		for(struct client *i=c;i!=NULL;i=i->next)
			state+=send_chat(i->s,msg);
	else
		return 1;
	return state;
}
int recv_chat(void *p) //服务器模式下，在子线程中运行，循环接受某个socket的客户端消息
{
	const int s=*(int *)p;
	char msg[INET6_ADDRSTRLEN+3+MSG_LENGTH]={0},*chat; //chat为去掉前缀的部分
	int state;
	int chat_peer=-1; //私聊的对方socket
	if(is_server)
		sprintf(msg,"[%s] ",lookup_client(s)->ip);
	chat=msg+strlen(msg);
	ready++;
	if(is_server)
	{
		sprintf(chat,"\033[1;33mJoined this chat as socket %d\033[0m\n",s);
		fputs(msg,stdout);
		send_chat(-1,msg);
	}
	while(ready)
	{
		state=recv(s,chat,MSG_LENGTH,0);
		if(state>0) //收到数据
		{
			chat[state]='\0';
			logmsg(0,is_server?"Received a message from server":"Received a message from %d",s);
			if(is_server)
			{
				//识别命令
				if(chat[0]=='/')
				switch(strscmp(chat,commands,sizeof(commands)/sizeof(char *)))
				{
					case 0:
						sscanf(chat,"/tell %d",&chat_peer);
						if(lookup_client(chat_peer)==NULL)
							chat_peer=-1;
					sprintf(chat,chat_peer<0?"Public chat\n":"Private chat to %d\n",chat_peer);
					chat_peer<0?(msg[0]='[',msg[chat-msg-2]=']'):(msg[0]='<',msg[chat-msg-2]='>');
					send_chat(s,msg);
					continue;
					case 1:
						current_users(s);
						continue;
					case 2:
						send_chat(s,"\033[1;37mConnected\033[0m\n");
						continue;
					default:
						send_chat(s,"\033[1;31mUnknown command\033[0m\n");
						continue;
				}
				if(send_chat(chat_peer,msg)<0)
				{
					chat_peer<0?logmsg(2,"Message forward error"):logmsg(2,"Message send to %d error",chat_peer);
					if(chat_peer>=0)
					{
						send_chat(s,"\033[1;33mMessage forward error, reset to public chat\033[0m\n");
						chat_peer=-1;
					}
				}
			}
			fputs(msg,stdout);
		}
		else //state的值不大于0，客户端已断开连接
		{
			if(state<0) //接收出错
			{
				logmsg(2,"Message read from %d error",s);
				perror("\033[1;33mSocket receive error\033[0m");
			}
			logmsg(1,"Peer %d disconnected",s);
			if(is_server)
			{
				disconnect_client(s);
				sprintf(chat,"\033[1;33mLeft this chat as socket %d\033[0m\n",s);
				fputs(msg,stdout);
				send_chat(-1,msg);
			}
			else
			{
				logmsg(2,"Disconnected from server");
				close(s),close_log();
				puts("Disconnected from server, exiting now.");
				if(ready>1)
					exit(0);
			}
			ready--;
			break;
		}
	}
	return 0;
}
