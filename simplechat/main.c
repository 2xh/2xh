//本文件包含运行程序的主函数和一些简化流程的函数实现，也包含了与其交互的部分量的定义
#include "simplechat.h" //部分量在此定义
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 4399 //未使用的端口
#define MAX_ERROR_COUNTS 3
union sockaddrs addr;
socklen_t addrlen;
int sock=-1; //sockfd
int is_server=0;
int ready=0;
char msg[MSG_LENGTH+1];
extern mtx_t mtx;
int strscmp(const char *s,char* const *cmp,const int n) //部分比较字符串，只要前部相同即可，返回首先比较成功的字符串位置，没有则返回-1
{
	for(int i=0,c;i<n;i++)
	{
		c=0;
		while(s[c]&&cmp[i][c])
		{
			if(s[c]!=cmp[i][c])
				break;
			c++;
		}
		if(!s[c]||!cmp[i][c])
			return i;
	}
	return -1;
}
void cleanup(void) //清理工作，包括关闭连接、关闭日志、销毁互斥体等
{
	if(ready)
		logmsg(2,"Force shutting down...");
	close_log();
	if(is_server)
		mtx_destroy(&mtx);
	if(sock>=0)
		close(sock);
}
int quickmsg(const char *msgname,const unsigned int n) //查找指定文件的指定行
{
	int i;
	FILE *msgfile;
	if((msgfile=fopen(msgname,"r"))==NULL)
	{
		perror("Failed to open msgfile");
		return -1;
	}
	for(i=0;i<n&&!feof(msgfile);i++)
		if(fgets(msg,MSG_LENGTH,msgfile)==NULL)
			break;
	fclose(msgfile);
	msg[MSG_LENGTH-1]='\n';
	if(i<n)
	{
		fputs("Too few lines\n",stderr);
		return -1;
	}
	else
		return 0;
}
int main(int argc,char** argv)
{
	//初始化
	char address[INET6_ADDRSTRLEN]={0}; //IPv4和IPv6相比，IPv6较长
	unsigned short port=PORT;
	init_log("/tmp/simplechat.log");
	//读入参数
	char* const argstr[]={"-h","--help","-?","-s","--server","-p","--port","-i","--ip"}; //可能的参数选项
	int argn=argc;
	char** argp=argv;
	while(--argn>0)
	{
		++argp;
		switch(strscmp(*argp,argstr,sizeof(argstr)/sizeof(char *))) //判定选项
		{
			case 0:case 1:case 2:
				cleanup();
				printf("Usage: %s [options]\n\nOptions:\n  -h, --help       Show this help and exit\n  -s, --server     Set mode to server\n  -p, --port PORT  Set port to PORT\n  -i, --ip ADDR    Set IP to ADDR, supports IPv4 and IPv6\n\nExit status:\n  1  Wrong input\n  0  Normal exit\n -1  Socket error\n -2  Connection error\n -3  Thread error\n\nFor command help, type /help in chat mode.\n",argv[0]);
				return 0;
			case 3:case 4:
				is_server=1,logmsg(1,"Set mode to server");
				continue;
			case 5:case 6:
				if(--argn>0)
				{
					port=atoi(*++argp);
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
				if(--argn>0)
				{
					strncpy(address,*++argp,INET6_ADDRSTRLEN-1),address[INET6_ADDRSTRLEN-1]='\0'; //使用strncpy，防止缓冲区溢出，防止程序出错
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
				fprintf(stderr,"Unrecognized parameter: %s, ignoring\n",*argp);
		}
	}
	if(is_server&&mtx_init(&mtx,mtx_plain)==thrd_error) //初始化互斥锁，防止大量客户端同时进出造成出错
	{
		logmsg(3,"Error setting up mtx lock");
		cleanup();
		fputs("Thread lock creation error, program cannot continue.\n",stderr);
		return -3;
	}
	//补充参数
	printf("Welcome to simplechat!\nCurrent mode: %s\n",is_server?"server":"client");
	if(!is_server&&!*address)
	{
		printf("Please provide an IP address to connect to: ");
		fgets(address,INET6_ADDRSTRLEN,stdin); //C11标准移除了gets()，只能用fgets()，但是可能会将\n包括在内
		if(address[strlen(address)-1]=='\n')
			address[strlen(address)-1]='\0';
		logmsg(1,"Set IP to %s",address);
	}
	printf("IP: %s, Port: %u\n",address[0]?address:"(Not specified)",port);
	//尝试将IP地址转换为IPv4或IPv6地址
	if(inet_pton(AF_INET,address,&addr.s4.sin_addr)>0)
		addr.s4.sin_family=AF_INET,addrlen=sizeof(addr.s4);
	else if(inet_pton(AF_INET6,address,&addr.s6.sin6_addr)>0)
		addr.s6.sin6_family=AF_INET6,addrlen=sizeof(addr.s6);
	else if(is_server)
		//没有参数，监听全部IPv6地址
		addr.s6.sin6_family=AF_INET6,addr.s6.sin6_addr=in6addr_any,addrlen=sizeof(addr.s6),logmsg(1,"No IP address or a wrong one is provided, binding to all IPv6 addresses");
	else
	{
		logmsg(3,"Neither IPv4 nor IPv6 address");
		cleanup();
		fputs("Wrong IP address, exiting now.\n",stderr);
		return 1;
	}
	if((sock=socket(addr.sa.sa_family,SOCK_STREAM,0))<0) //TCP连接
	{
		logmsg(3,"Failed to create socket");
		cleanup();
		perror("Socket creation failed");
		fputs("Exiting now.\n",stderr);
		return -1;
	}
	addr.s4.sin_port=htons(port);
	int sockopt=1;
	setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE|SO_REUSEADDR,&sockopt,sizeof(int)); //设置socket，启用KeepAlive功能
	if(is_server&&bind(sock,&addr.sa,addrlen)<0)
	{
		logmsg(3,"Failed to bind socket");
		cleanup();
		perror("Socket bind failed");
		fprintf(stderr,"Please ensure this device owns IP %s and port %u is not occupied.\nBinding ports below 1024 requires privilege.\nExiting now.\n",address,port);
		return -1;
	}
	//连接，最多尝试MAX_ERROR_COUNTS次
	int err_count=0;
	connect:
	if(is_server)
		if(listen(sock,MAX_CLIENTS)<0)
		{
			logmsg(3,"Failed to listen on socket");
			perror("Failed to listen on socket");
			fputs("Please use command to try again.\n",stderr);
		}
		else
		{
			logmsg(1,"Socket listening");
			puts("Ready to receive connection");
		}
	else
	{
		logmsg(1,"Connecting to server");
		fputs("Trying to connect, this may take some time...\n",stderr);
		if(connect(sock,&addr.sa,addrlen)<0)
		{
			logmsg(3,"Failed to connect");
			perror("Failed to connect to server");
			err_count++;
			if(err_count>=MAX_ERROR_COUNTS)
			{
				logmsg(2,"Max retry times reached");
				cleanup();
				fprintf(stderr,"%s seems unreachable.\nPlease check both network status and firewall settings.\nExiting now.\n",address);
				return -2;
			}
			else
				fputs("Retrying now.\n",stderr);
			goto connect;
		}
		else
		{
			logmsg(1,"Connected");
			puts("Ready to chat");
		}
	}
	ready=1;
	//创建线程
	thrd_t chat_th;
	if(is_server&&thrd_create(&chat_th,accept_client,&sock)!=thrd_success) //服务端创建接受客户端连接的线程
	{
		logmsg(3,"Failed to create thread");
		fputs("Chat server not available due to thread error\n",stderr);
	}
	if(!is_server&&thrd_create(&chat_th,recv_chat,&sock)!=thrd_success) //客户端创建接收服务端消息的线程
	{
		logmsg(3,"Failed to create thread");
		fputs("Chat client not available due to thread error\n",stderr);
	}
	//输入内容或命令
	char* const commands[]={"/help","/list","/qmsg","/exit"};
	int msg_no;
	while(ready&&!feof(stdin))
	{
		if(fgets(msg,MSG_LENGTH,stdin)==NULL) //C11标准移除了gets()，只能用fgets()
			continue;
		msg[MSG_LENGTH-1]='\n';
		if(msg[0]=='/')
			switch(strscmp(msg,commands,sizeof(commands)/sizeof(char *)))
			{
				case 0:
					puts("Available commands:\n  /help         Display this help\n  /list         List clients in the chat, current client is marked with *");
					if(!is_server)
						puts("  /tell SOCKET  When SOCKET>=0:\n                  Set chat mode to private with client whose socket is SOCKET (ONE-WAY)\n                When SOCKET<0:\n                  Set chat mode to public\n  /ping         Test network connectivity");
					puts("  /qmsg N       Send with message in line N, requires msg.txt to be placed in the same directory\n  /exit         Leave the chat and exit\n\nNote:\n  No echo message is sent back in private chat, and the peer must specify before chatting to the sender.\n  To exit more quickly, use EOF (Ctrl+D).");
					continue;
				case 1:
					if(!is_server)
						break;
					current_users(-1);
					continue;
				case 2:
					if(sscanf(msg,"/qmsg %d",&msg_no)<1)
					{
						puts("Invalid command");
						continue;
					}
					if(quickmsg("msg.txt",msg_no)==0)
						break;
					continue;
				case 3:
					ready=0;
					continue;
				default:
					if(!is_server)
						break;
					puts("Unknown command");
					continue;
			}
		if(is_server)
		{
			send_chat(-1,"[Server] ");
			printf("[Server] %s",msg);
		}
		send_chat(is_server?-1:sock,msg);
	}
	cleanup();
	puts("Bye!");
	return 0;
}
