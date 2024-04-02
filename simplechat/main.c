#include "simplechat.h"
#include "logsystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int main(int argc,char **argv)
{
	int is_server=0,ip_protocol=0;
	char addr[INET6_ADDRSTRLEN];
	unsigned short port=4399; //未使用的端口
	init_log(NULL);
	while(--argc>0)
	{
		++argv;
		if(!strcmp(*argv,"-h")||!strcmp(*argv,"--help"))
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
			port=atoi((--argc,*++argv)),logmsg(1,"Set port to %d",port);
			continue;
		}
		fprintf(stderr,"Unrecognized parameter: %s, ignoring\n",*argv);
	}
	close_log();
	return 0;
}
