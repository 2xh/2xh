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
	char *msg="SOCKET  PORT IP\n";
	struct client *i=c;
	s>=0?send_chat(s,msg):fputs(msg,stdout);
	msg=(char *)malloc(sizeof(char)*60);
	while(i!=NULL)
	{
		count++;
		sprintf(msg,"%6d %5u %s\n",i->s,i->port,i->ip);
		s>=0?send_chat(s,msg):fputs(msg,stdout);
		i=i->next;
	}
	sprintf(msg,"Total: %u",count);
	s>=0?send_chat(s,msg):puts(msg);
	free(msg);
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
		if(send(s,msg,strlen(msg)+1,0)<0)
		{
			logmsg(2,"Message send to %d error",s);
			return -1;
		}
		is_server?logmsg(0,"A message has been sent to %d",s):logmsg(0,"A message has been sent to server");
	}
	else
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
	char msg[MSG_LENGTH];
	char prefix[INET6_ADDRSTRLEN+3];
	int state;
	int chat_peer=-1;
	while(ready)
	{
		state=recv(s,msg,MSG_LENGTH-1,0);
		if(state<0)
			logmsg(2,"Message read from %d error",s);
		else if(state>0)
		{
			if(is_server)
			{
				logmsg(0,"Received a message from %d",s);
				if(msg[0]=='/')
					switch(strscmp(msg,commands,sizeof(commands)/sizeof(char *)))
					{
						case 0:
							if(sscanf(msg,"/tell %d",&chat_peer)==1)
								send_chat(s,chat_peer<0?"[Server] Public chat enabled":"[Server] Private chat enabled");
							continue;
						case 1:
							current_users(s);
							continue;
						default:
							send_chat(s,"Unknown command");
							continue;
					}
				printf("<%s> ",lookup_client(s)->ip);
				sprintf(prefix,"[%s] ",lookup_client(s)->ip);
				send_chat(chat_peer,prefix);
				send_chat(chat_peer,msg);
			}
			else
				logmsg(0,"Received a message from server");
			printf("%s\n",msg);
		}
		else
		{
			logmsg(1,"Peer %d disconnected",s);
			if(is_server)
				disconnect_client(s);
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
