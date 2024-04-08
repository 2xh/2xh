#ifndef _LOGSYSTEM_H
#define _LOGSYSTEM_H
int init_log(const char *logname);
int close_log(void);
int logmsg(const int type,const char *msg,...);
#endif
