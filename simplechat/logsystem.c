#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#define LOG_LEVEL 1
int logready=0;
int log_to_file=1;
FILE *logfile;
int init_log(const char *logname)
{
	if(logready)
	{
		fputs("Log system is already started\n",stderr);
		return 1;
	}
	if(log_to_file=logname!=NULL)
		fprintf(stderr,"Log file set to %s\n",logname);
	logfile=log_to_file?fopen(logname,"a"):stderr;
	if(logfile==NULL)
	{
		perror("Failed to create log file");
		fputs("Redirecting to stderr\n",stderr);
		init_log(NULL);
		return -1;
	}else{
		fputs("Log system started\n",stderr);
		logready=1;
		return 0;
	}
}
int close_log(void)
{
	if(!logready)
	{
		fputs("Log system is already stopped\n",stderr);
		return 1;
	}
	if(log_to_file&&fclose(logfile)==EOF)
	{
		perror("Failed to close log file");
		return -1;
	}else{
		fputs("Log system stopped\n",stderr);
		logready=0;
		return 0;
	}
}
int logmsg(const int type,const char *msg,...)
{
	time_t t=time(NULL);
	char time_prefix[11];
	char *type_prefix,*color_prefix;
	va_list args;
	if(!logready)
		return -1;
	if(type<LOG_LEVEL)
		return 1;
	if(strftime(time_prefix,11,"[%X]",localtime(&t)))
		fprintf(logfile,"%s ",time_prefix);
	switch(type)
	{
		case 0:color_prefix="",type_prefix="DEBUG";break;
		case 1:color_prefix="\033[1;37m",type_prefix="INFO";break;
		case 2:color_prefix="\033[1;33m",type_prefix="WARN";break;
		case 3:color_prefix="\033[1;31m",type_prefix="ERROR";break;
		default:color_prefix="",type_prefix="UNKNOWN";
	}
	va_start(args,msg);
	print_log:
	if(fprintf(logfile,"%s[%s] ",log_to_file?"":color_prefix,type_prefix),vfprintf(logfile,msg,args),fprintf(logfile,"%s\n",log_to_file?"":"\033[0m")<0)
	{
		perror("Error writing log");
		close_log();
		fputs("Restarting log system\n",stderr);
		init_log(NULL);
		goto print_log;
	}
	va_end(args);
	return 0;
}
