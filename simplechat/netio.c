#include "simplechat.h"
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <sys/socket.h>
#include <unistd.h>
extern socklen_t addrlen;
extern int ready,is_server;
struct client {
	int s;
	char ip[INET6_ADDRSTRLEN];
	unsigned short port;
	struct client *next;
} *c=NULL;
mtx_t mtx;
char* const commands[]={"/tell","/list"};
struct client* lookup_client(const int s)
{
	struct client *i=c;
	while(i!=NULL)
	{
		if(i->s==s)
			return i;
		else
			i=i->next;
	}
	return NULL;
}
unsigned int current_users(const int s)
{
	unsigned int count=0;
	char *head="SOCKET  PORT IP\n";
	char msg[14+INET6_ADDRSTRLEN];
	struct client *i=c;
	s>=0?send_chat(s,head):fputs(head,stdout);
	while(i!=NULL)
	{
		count++;
		sprintf(msg,"%c%5d %5u %s\n",s==i->s?'*':' ',i->s,i->port,i->ip);
		s>=0?send_chat(s,msg):fputs(msg,stdout);
		i=i->next;
	}
	sprintf(msg,"Total: %u\n",count);
	s>=0?send_chat(s,msg):puts(msg);
	return count;
}
int accept_client(void *p)
{
	const int sock=*(int *)p;
	int s;
	thrd_t th;
	struct client *t,*i;
	union sockaddrs addr;
	while(ready)
	{
		if((s=accept(sock,&addr.sa,&addrlen))>0)
		{
			if(mtx_lock(&mtx)==thrd_error)
				logmsg(2,"mtx_lock failed");
			t=(struct client *)malloc(sizeof(struct client));
			t->s=s,t->next=NULL;
			if(getpeername(t->s,&addr.sa,&addrlen)<0)
			{
				logmsg(2,"Unknown remote address on socket %d",t->s);
				t->ip[0]='\0';
				t->port=0;
			}
			else
			{
				if(inet_ntop(addr.sa.sa_family,addr.sa.sa_family==AF_INET6?&addr.s6.sin6_addr:&addr.s4.sin_addr,t->ip,sizeof(t->ip))==NULL)
					logmsg(2,"Invalid remote IP address");
				t->port=ntohs(addr.s4.sin_port);
				logmsg(1,"Remote address: [%s]:%u",t->ip,t->port);
			}
			if(c==NULL)
				c=t;
			else
			{
				i=c;
				while(i->next!=NULL)
					i=i->next;
				i->next=t;
			}
			mtx_unlock(&mtx);
			if(thrd_create(&th,recv_chat,&(t->s))!=thrd_success&&thrd_detach(th))
			{
				logmsg(3,"Failed to create thread");
				fputs("Chat server is not stable due to thread error\n",stderr);
			}
			logmsg(1,"Added a new client %d",t->s);
		}
	}
	return 0;
}
int disconnect_client(const int s)
{
	struct client *i,*t;
	if(close(s)<0)
		logmsg(2,"Socket %d close failed",s);
	if(!is_server||c==NULL)
		return -1;
	else
	{
		if(mtx_lock(&mtx)==thrd_error)
			logmsg(2,"mtx_lock failed");
		i=c;
		if(i->s==s)
		{
			c=i->next;
			free(i);
		}
		else
			while(i->next!=NULL)
			{
				if(i->next->s==s)
				{
					t=i->next;
					i->next=t->next;
					free(t);
					break;
				}
				else
					i=i->next;
			}
		mtx_unlock(&mtx);
	}
	logmsg(1,"Removed client %d",s);
	return 0;
}
int send_chat(const int s,const char* msg)
{
	if(s>=0)
	{
		if(send(s,msg,strlen(msg),0)<0)
		{
			logmsg(2,"Message send to %d error",s);
			return -1;
		}
		is_server?logmsg(0,"A message has been sent to %d",s):logmsg(0,"A message has been sent to server");
	}
	else if(is_server)
	{
		struct client *i=c;
		while(i!=NULL)
		{
			send_chat(i->s,msg);
			i=i->next;
		}
	}
	return 0;
}
int recv_chat(void *p)
{
	const int s=*(int *)p;
	char msg[INET6_ADDRSTRLEN+3+MSG_LENGTH]={0},*chat;
	int state;
	int chat_peer=-1;
	if(is_server)
		sprintf(msg,"[%s] ",lookup_client(s)->ip);
	chat=msg+strlen(msg);
	if(is_server)
	{
		sprintf(chat,"\033[1;33mJoined this chat as socket %d\033[0m\n",s);
		printf(msg);
		send_chat(-1,msg);
	}
	while(ready)
	{
		state=recv(s,chat,MSG_LENGTH,0);
		if(state<0)
			logmsg(2,"Message read from %d error",s);
		else if(state>0)
		{
			chat[state]='\0';
			if(is_server)
			{
				logmsg(0,"Received a message from %d",s);
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
						default:
							send_chat(s,"Unknown command\n");
							continue;
					}
				if(send_chat(chat_peer,msg)<0)
				{
					chat_peer<0?logmsg(2,"Message forward error"):logmsg(2,"Message send to %d error",chat_peer);
					return -1;
				}
			}
			else
				logmsg(0,"Received a message from server");
			printf(msg);
		}
		else
		{
			logmsg(1,"Peer %d disconnected",s);
			if(is_server)
			{
				disconnect_client(s);
				sprintf(chat,"\033[1;33mLeft this chat as socket %d\033[0m\n",s);
				printf(msg);
				send_chat(-1,msg);
			}
			else
			{
				logmsg(2,"Disconnected from server");
				close_log();
				close(s);
				puts("Disconnected from server, exiting now.");
				exit(0);
			}
			break;
		}
	}
	return 0;
}
