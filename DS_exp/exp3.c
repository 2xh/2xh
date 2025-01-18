#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
struct stack {
	union {
		void* data;
		double* ddata;
		char* cdata;
	};
	size_t length;
};
bool ra(struct stack* l,size_t newlen) //reallocate memory
{
	if(!newlen)
		free(l->data),l->data=NULL;
	else if(!l->data)
	{
		if(!(l->data=malloc(newlen)))
			return false;
	}
	else
	{
		void* p=realloc(l->data,newlen);
		if(!p)
			return false;
		l->data=p;
	}
	return true;
}
bool pushd(struct stack* l,double data)
{
	if(!ra(l,(l->length+1)*sizeof(double)))
		return false;
	l->ddata[l->length++]=data;
	return true;
}
bool pushc(struct stack* l,char data)
{
	if(!ra(l,(l->length+1)*sizeof(char)))
		return false;
	l->cdata[l->length++]=data;
	return true;
}
double topd(struct stack* l)
{
	return l->length?l->ddata[l->length-1]:nan("");
}
char topc(struct stack* l)
{
	return l->length?l->cdata[l->length-1]:0;
}
double popd(struct stack* l)
{
	double x;
	if(!l->length)
		return nan("");
	x=l->ddata[--l->length];
	ra(l,l->length*sizeof(double));
	return x;
}
char popc(struct stack* l)
{
	char x;
	if(!l->length)
		return 0;
	x=l->cdata[--l->length];
	ra(l,l->length*sizeof(char));
	return x;
}
unsigned char priority(char c)
{
	switch(c)
	{
		case '(':return 0;
		case '^':return 1;
		case '*':case '/':return 2;
		case '+':case '-':return 3;
		case ')':return 15;
		default:return -1;
	}
}
double x;
char c;
#define NUM 0
#define SIG 1
int type;
void docalc(struct stack* n,struct stack* s)
{
	double a,b;
	while(s->length&&priority(c)>priority(topc(s)))
	{
		b=popd(n),a=popd(n);
		switch(popc(s))
		{
			case '+':pushd(n,a+b);break;
			case '-':pushd(n,a-b);break;
			case '*':pushd(n,a*b);break;
			case '/':pushd(n,a/b);break;
			case '^':pushd(n,pow(a,b));break;
		}
	}
}
double mid_calc(void)
{
	double d;
	struct stack n={0},s={0};
	type=SIG;
	pushd(&n,0);
	while(type!=EOF)
	{
		if(scanf("%1[-+]",&c)==1)
			docalc(&n,&s),pushc(&s,c),type=SIG;
		else if(type==SIG&&scanf("%lf",&x)==1)
			pushd(&n,x),type=NUM;
		else
		{
			if((c=getchar())==EOF)
				c=')';
			docalc(&n,&s);
			if(c==')'||c=='\n')
			{
				d=popd(&n);
				ra(&n,0),ra(&s,0);
				type=EOF;
			}
			else if(c=='(')
				pushd(&n,mid_calc()),type=NUM;
			else pushc(&s,c),type=SIG;
		}
	}
	return d;
}
double rear_calc(void)
{
	double d;
	struct stack n={0},s={0};
	do
	{
		if(scanf("%lf",&x)==1)
			pushd(&n,x);
		if((c=getchar())!=EOF&&priority(c)!=(unsigned char)-1)
			pushc(&s,c),c=')',docalc(&n,&s);
	}
	while(c!=EOF&&scanf("%1[\n]",&c)==0);
	d=popd(&n);
	ra(&n,0),ra(&s,0);
	return d;
}
int main(void)
{
	printf("%.15g\n",mid_calc());
	return 0;
}
