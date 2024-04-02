#ifndef _SIMPLECHAT_H
#define _SIMPLECHAT_H
int listen_chat(void);
int send_chat(char* ip,char* msg);
int current_users(void);
int quickmsg(int n);
#endif
