#ifndef _LOGSYSTEM_H
#define _LOGSYSTEM_H
int init_log(char *logname);
int close_log(void);
int logmsg(int type,char *msg,...);
#endif
