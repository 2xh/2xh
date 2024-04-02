#include "simplechat.h"
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 4399 //未使用的端口
union sockaddrs {
	struct sockaddr sa;
	struct sockaddr_in s4;
	struct sockaddr_in6 s6;
} addr;
socklen_t addrlen;
int s; //socket handler
int main(int argc,char **argv)
{
	//初始化
	int is_server=0;
	char address[INET6_ADDRSTRLEN]={0};
	unsigned short port=PORT;
	init_log(NULL);
	//读入参数
	while(--argc>0)
	{
		++argv;
		if(!strcmp(*argv,"-h")||!strcmp(*argv,"--help")||!strcmp(*argv,"-?"))
		{
			close_log();
			puts(""); //help
			return 0;
		}
		if(!strcmp(*argv,"-s")||!strcmp(*argv,"--server"))
		{
			is_server=1,logmsg(1,"Set mode to server");
			continue;
		}
		if(!strcmp(*argv,"-p")||!strcmp(*argv,"--port"))
		{
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
				close_log();
				fputs("Insufficient parameters, exiting now.\n",stderr);
				return 1;
			}
		}
		if(!strcmp(*argv,"-i")||!strcmp(*argv,"--ip"))
		{
			if(--argc>0)
			{
				strncpy(address,*++argv,INET6_ADDRSTRLEN-1),address[INET6_ADDRSTRLEN-1]='\0';
				logmsg(1,"Set IP to %s",address);
				continue;
			}
			else
			{
				close_log();
				fputs("Insufficient parameters, exiting now.\n",stderr);
				return 1;
			}
		}
		fprintf(stderr,"Unrecognized parameter: %s, ignoring\n",*argv);
	}
	//补充参数
	puts("Welcome to chatroom_demo!");
	if(!is_server&&!*address)
	{
		printf("Please provide an IP address to connect to: ");
		fgets(address,INET6_ADDRSTRLEN,stdin); //C11标准移除了gets()，只能用fgets()，但是可能会将行尾空格包括在内
		if(address[strlen(address)-1]=='\n')
			address[strlen(address)-1]='\0';
		logmsg(1,"Set IP to %s",address);
	}
	if(inet_pton(AF_INET,address,&addr.s4.sin_addr)>0)
		addr.s4.sin_family=AF_INET,addrlen=sizeof(addr.s4);
	else if(inet_pton(AF_INET6,address,&addr.s6.sin6_addr)>0)
		addr.s6.sin6_family=AF_INET6,addrlen=sizeof(addr.s6);
	else if(is_server)
		addr.s4.sin_addr.s_addr=INADDR_ANY,addr.s4.sin_family=AF_INET,addrlen=sizeof(addr.s4),logmsg(1,"No IP address or a wrong one is provided, binding to all IPv4 addresses");
	else
	{
		logmsg(3,"Neither IPv4 nor IPv6 address");
		close_log();
		fputs("Wrong IP address, exiting now.\n",stderr);
		return 1;
	}
	if((s=socket(addr.sa.sa_family,SOCK_STREAM,0))<0)
	{
		logmsg(3,"Failed to create socket");
		close_log();
		fputs("Socket creation failed, exiting now.\n",stderr);
		return -1;
	}
	addr.s4.sin_port=htons(port);
	if(bind(s,&addr.sa,addrlen)<0)
	{
		logmsg(3,"Failed to bind socket");
		close_log();
		fprintf(stderr,"Socket bind failed. Please make sure IP %s is correct and port %u is not occupied.\nBinding ports below 1024 requires privilege.\nExiting now.\n",address,port);
		return -1;
	}
	//separate client and server
	close_log();
	return 0;
}
