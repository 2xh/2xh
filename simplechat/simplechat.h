#ifndef _SIMPLECHAT_H
#define _SIMPLECHAT_H
#include <arpa/inet.h>
#define MAX_CLIENTS 100
#define MSG_LENGTH 255
union sockaddrs {
	struct sockaddr sa;
	struct sockaddr_in s4;
	struct sockaddr_in6 s6;
};
int accept_client(void *);
int send_chat(const int s,const char* msg);
int recv_chat(void *s);
int disconnect_client(const int s);
unsigned int current_users(const int s);
int quickmsg(const char *msgname,const unsigned int n);
int strscmp(const char *s,char* const *cmp,const int n);
#endif
