#include <stdio.h>
#include <stdlib.h>
struct list {
	unsigned data;
	unsigned n;
	struct list *next;
};
struct list* push(struct list *q,unsigned x)
{
	struct list *p;
	if(!(p=(struct list*)malloc(sizeof(struct list))))
		return NULL;
	p->data=x;
	if(q)
		p->next=q->next,q->next=p;
	else
		p->next=p;
	return p;
}
struct list* pop(struct list *q)
{
	struct list *p;
	if(!q||!q->next)
		return NULL;
	p=q->next;
	if(p==q)
		q=NULL;
	else
		q->next=p->next;
	free(p);
	return q;
}
char c=0;
int main(void)
{
	unsigned m,n=0,p;
	struct list *q=NULL;
	scanf("%u",&m);
	while(c!='\n'&&scanf("%u%c",&p,&c)>0)
		q=push(q,p),q->n=++n;
	while(q)
	{
		for(;m>1;--m)
			q=q->next;
		printf("%u ",q->next->n);
		m=q->next->data;
		q=pop(q);
	}
	printf("\n");
	return 0;
}
