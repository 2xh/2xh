#include "simplechat.h"
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <threads.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 4399 //未使用的端口
#define MAX_ERROR_COUNTS 3
union sockaddrs addr;
socklen_t addrlen;
int sock=-1; //sockfd
int is_server=0;
int ready=0;
char msg[MSG_LENGTH];
extern mtx_t mtx;
int strscmp(const char *s,char* const *cmp,const int n)
{
	for(int i=0;i<n;i++)
		if(!strcmp(s,*(cmp+i)))
			return i;
	return -1;
}
void cleanup(void)
{
	if(ready)
		logmsg(2,"Force shutting down...");
	close_log();
	if(is_server)
		mtx_destroy(&mtx);
	if(sock>=0)
		close(sock);
}
int main(int argc,char **argv)
{
	//初始化
	char address[INET6_ADDRSTRLEN]={0};
	unsigned short port=PORT;
	init_log(NULL);
	//读入参数
	char* const argstr[]={"-h","--help","-?","-s","--server","-p","--port","-i","--ip"};
	while(--argc>0)
	{
		++argv;
		switch(strscmp(*argv,argstr,sizeof(argstr)/sizeof(char *)))
		{
			case 0:case 1:case 2:
				cleanup();
				puts(""); //help
				return 0;
			case 3:case 4:
				is_server=1,logmsg(1,"Set mode to server");
				continue;
			case 5:case 6:
				if(--argc>0)
				{
					port=atoi(*++argv);
					if(port>0)
						logmsg(1,"Set port to %u",port);
					else
						port=PORT,logmsg(2,"Invalid port, reseting to default (%u)",port);
					continue;
				}
				else
				{
					cleanup();
					fputs("Insufficient parameters, exiting now.\n",stderr);
					return 1;
				}
			case 7:case 8:
				if(--argc>0)
				{
					strncpy(address,*++argv,INET6_ADDRSTRLEN-1),address[INET6_ADDRSTRLEN-1]='\0';
					logmsg(1,"Set IP to %s",address);
					continue;
				}
				else
				{
					cleanup();
					fputs("Insufficient parameters, exiting now.\n",stderr);
					return 1;
				}
			default:
				fprintf(stderr,"Unrecognized parameter: %s, ignoring\n",*argv);
		}
	}
	if(is_server&&mtx_init(&mtx,mtx_plain)==thrd_error)
	{
		logmsg(3,"Error setting up mtx lock");
		cleanup();
		fputs("Thread lock creation error, program cannot continue.\n",stderr);
		return -3;
	}
	//补充参数
	puts("Welcome to chatroom_demo!");
	if(!is_server&&!*address)
	{
		printf("Please provide an IP address to connect to: ");
		fgets(address,INET6_ADDRSTRLEN,stdin); //C11标准移除了gets()，只能用fgets()，但是可能会将\n包括在内
		if(address[strlen(address)-1]=='\n')
			address[strlen(address)-1]='\0';
		logmsg(1,"Set IP to %s",address);
	}
	if(inet_pton(AF_INET,address,&addr.s4.sin_addr)>0)
		addr.s4.sin_family=AF_INET,addrlen=sizeof(addr.s4);
	else if(inet_pton(AF_INET6,address,&addr.s6.sin6_addr)>0)
		addr.s6.sin6_family=AF_INET6,addrlen=sizeof(addr.s6);
	else if(is_server)
		addr.s4.sin_addr.s_addr=INADDR_ANY,addr.s4.sin_family=AF_INET,addrlen=sizeof(addr.s4),logmsg(2,"No IP address or a wrong one is provided, binding to all IPv4 addresses");
	else
	{
		logmsg(3,"Neither IPv4 nor IPv6 address");
		cleanup();
		fputs("Wrong IP address, exiting now.\n",stderr);
		return 1;
	}
	if((sock=socket(addr.sa.sa_family,SOCK_STREAM,0))<0)
	{
		logmsg(3,"Failed to create socket");
		cleanup();
		fputs("Socket creation failed, exiting now.\n",stderr);
		return -1;
	}
	addr.s4.sin_port=htons(port);
	if(is_server&&bind(sock,&addr.sa,addrlen)<0)
	{
		logmsg(3,"Failed to bind socket");
		cleanup();
		fprintf(stderr,"Socket bind failed. Please make sure IP %s is correct and port %u is not occupied.\nBinding ports below 1024 requires privilege.\nExiting now.\n",address,port);
		return -1;
	}
	//连接
	int err_count=0;
	connect:
	if(is_server)
		if(listen(sock,MAX_CLIENTS)<0)
		{
			logmsg(3,"Failed to listen");
			fputs("Failed to listen on socket, entering offline mode.\nPlease use command to try again.\n",stderr);
		}
		else
		{
			logmsg(1,"Socket listening");
			fputs("Ready to receive connection\n",stderr);
		}
	else
	{
		logmsg(1,"Connecting to server");
		fputs("Trying to connect, this may take some time...\n",stderr);
		if(connect(sock,&addr.sa,addrlen)<0)
		{
			logmsg(3,"Failed to connect");
			err_count++;
			if(err_count>=MAX_ERROR_COUNTS)
			{
				logmsg(2,"Max retry times reached");
				cleanup();
				fprintf(stderr,"%s seems unreachable.\nPlease check both network status and firewall settings.\nExiting now.\n",address);
				return -2;
			}
			else
				fprintf(stderr,"Retrying now.\n");
			goto connect;
		}
		else
		{
			logmsg(1,"Connected");
			fprintf(stderr,"Ready to chat\n");
		}
	}
	ready=1;
	//创建线程
	thrd_t chat_th;
	if(is_server&&thrd_create(&chat_th,accept_client,NULL)!=thrd_success)
	{
		logmsg(3,"Failed to create thread");
		fputs("Chat server not available due to thread error\n",stderr);
	}
	if(!is_server&&thrd_create(&chat_th,recv_chat,&sock)!=thrd_success)
	{
		logmsg(3,"Failed to create thread");
		fputs("Chat client not available due to thread error\n",stderr);
	}
	char* const commands[]={"/help","/list","/qmsg","/reconnect","/exit"};
	while(ready)
	{
		fgets(msg,MSG_LENGTH,stdin); //C11标准移除了gets()，只能用fgets()，但是可能会将\n包括在内
		if(msg[strlen(msg)-1]=='\n')
			msg[strlen(msg)-1]='\0';
		if(msg[0]=='/')
			switch(strscmp(msg,commands,sizeof(commands)/sizeof(char *)))
			{
				case 0:
					//help
					continue;
				case 1:
					//list
					continue;
				case 2:
					//qmsg
					continue;
				case 3:
					goto connect;
				case 4:
					ready=0;
					continue;
				default:
					fputs("Unknown command\n",stderr);
					continue;
			}
		send_chat(is_server?-1:sock,msg);
	}
	cleanup();
	fputs("Bye!\n",stderr);
	return 0;
}
