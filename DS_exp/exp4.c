#include <stdio.h>
#include <stdlib.h>
/*#include <string.h>*/
#include <stdbool.h>
bool *maze,*route;
unsigned m=0,n=0,om=0,on=0;
bool findmaze(unsigned cm,unsigned cn)
{
	unsigned d=0;
	if(cm>=m||cn>=n||maze[cm*n+cn]||route[cm*n+cn])
		return false;
	route[cm*n+cn]=true;
	if(!(cm==om&&cn==on))
	{
		++d;
		if(!findmaze(cm,cn+1))
		{
			++d;
			if(!findmaze(cm+1,cn))
			{
				++d;
				if(!findmaze(cm,cn-1))
				{
					++d;
					if(!findmaze(cm-1,cn))
					{
						route[cm*n+cn]=false;
						return false;
					}
				}
			}
		}
	}
	printf("(%u,%u,%u)\n",cm+1,cn+1,d);
	return true;
}
void findallmaze(unsigned cm,unsigned cn)
{
	unsigned d=0;
	if(cm>=m||cn>=n||maze[cm*n+cn]||route[cm*n+cn])
		return;
	route[cm*n+cn]=true;
	if(cm==om&&cn==on)
	{
		for(unsigned i=0;i<m*n;i++)
		{
			printf("%c",route[i]?'*':maze[i]+'0');
			if(i%n==n-1)
				printf("\n");
		}
		printf("\n");
	}
	else
	{
		++d;
		findallmaze(cm,cn+1);
		++d;
		findallmaze(cm+1,cn);
		++d;
		findallmaze(cm,cn-1);
		++d;
		findallmaze(cm-1,cn);
	}
	route[cm*n+cn]=false;
}
int main()
{
	unsigned im=0,in=0;
	scanf("%u%u",&m,&n);
	if(!(maze=(bool*)malloc(sizeof(bool)*(m*n)))||!(route=(bool*)calloc(sizeof(bool),m*n)))
		perror("OOM"),abort();
	for(unsigned i=0;i<m*n;++i)
		scanf("%hhu",maze+i);
	scanf("%u%u%u%u",&im,&in,&om,&on);
	im-=1,in-=1,om-=1,on-=1;
	/*if(findmaze(im,in))
		for(unsigned i=0;i<m*n;i++)
		{
			printf("%c",route[i]?'*':maze[i]+'0');
			if(i%n==n-1)
				printf("\n");
		}
	else
		puts("Not found");
	memset(route,0,m*n);*/
	findallmaze(im,in);
	return 0;
}
