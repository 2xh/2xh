#include <stdio.h>
#include <stdlib.h>
const char* OOM="Allocation failed",*WI="Illegal input";
struct term {
	double c;
	int e;
	struct term *next;
};
void free_all(struct term *p)
{
	for(struct term *q=p;p;q=p)
		p=p->next,free(q);
}
struct term* free_zero(struct term *p)
{
	struct term *q,*i;
	if(!p)
		return NULL;
	while(p&&p->c==0)
		q=p,p=p->next,free(q);
	for(q=p;q&&q->next;q=q->next)
		if(q->next->c==0)
			i=q->next,q->next=q->next->next,free(i);
	return p;
}
struct term* insert_term(struct term *p,double c,int e)
{
	struct term *q=p,*i=(struct term*)malloc(sizeof(struct term));
	if(!i)
	{
		perror(OOM);
		return NULL;
	}
	i->c=c,i->e=e,i->next=NULL;
	if(!p)
		return i;
	if(e>p->e)
	{
		i->next=p;
		return i;
	}
	while(q->next&&e<q->next->e)
		q=q->next;
	i->next=q->next,q->next=i;
	return p;
}
struct term* read_terms(void)
{
	double c;
	int e,s;
	char ch=0;
	struct term *p=NULL,*i;
	while(ch!='\n'&&(s=scanf("%lf %d%c",&c,&e,&ch))!=EOF)
	{
		if(s>=2)
		{
			i=insert_term(p,c,e);
			if(!i)
				return p;
			else
				p=i;
		}
		else
		{
			fprintf(stderr,"%s\n",WI);
			scanf("%*[^\n]");
			continue;
		}
	}
	return p;
}
void scalar_mul(struct term* p,double n)
{
	while(p)
		p->c*=n,p=p->next;
}
void print_terms(struct term* p)
{
	if(!p)
		printf("0");
	else while(p)
	{
		printf("%+.15g",p->c);
		if(p->c==1||p->c==-1)
			printf("\b");
		if(p->e)
			printf("x^%d",p->e);
		p=p->next;
	}
	printf("\n");
}
struct term* add_sorted(const struct term *a,const struct term *b)
{
	struct term *c=NULL,*d;
	while(a||b)
	{
		if(a&&b&&a->e>=b->e||!b)
		{
			if(!c)
				c=d=insert_term(NULL,a->c,a->e);
			else if(a->e==d->e)
				d->c+=a->c;
			else if(!insert_term(d,a->c,a->e))
			{
				free_all(c);
				return NULL;
			}
			else
				d=d->next;
			a=a->next;
		}
		else
		{
			if(!c)
				c=d=insert_term(NULL,b->c,b->e);
			else if(b->e==d->e)
				d->c+=b->c;
			else if(!insert_term(d,b->c,b->e))
			{
				free_all(c);
				return NULL;
			}
			else
				d=d->next;
			b=b->next;
		}
	}
	return free_zero(c);
}
struct term* term_mul(const struct term* a,const struct term* b)
{
	struct term *c=NULL,*d,*e,*f,*g;
	while(a)
	{
		e=NULL;
		for(d=b;d;d=d->next)
		{
			if(!e)
				e=f=insert_term(NULL,a->c*d->c,a->e+d->e);
			else if(!insert_term(f,a->c*d->c,a->e+d->e))
			{
				free_all(c),free_all(e);
				return NULL;
			}
			else
				f=f->next;
		}
		g=add_sorted(c,e);
		free_all(c),free_all(e);
		c=g;
		a=a->next;
	}
	return c;
}
double bitpow(double x,int y)
{
	double z=1;
	for(unsigned a=abs(y);a;x*=x,a>>=1)
		if(a&1)
			z*=x;
	if(y<0)
		z=1.0/z;
	return z;
}
double value(const struct term* p,double x)
{
	double y=0;
	int e=0;
	while(p)
		y+=p->c*bitpow(x,p->e-e),e=p->e,p=p->next;
	return y;
}
int main(void)
{
	struct term *a,*b,*c;
	double x=0;
	a=read_terms(),b=read_terms();
	scanf("%lf",&x);
	c=add_sorted(a,b);
	printf("A+B=");
	print_terms(c);
	printf("%.15g\n",value(c,x));
	free_all(c);
	c=term_mul(a,b);
	printf("A*B=");
	print_terms(c);
	printf("%.15g\n",value(c,x));
	free_all(c);
	scalar_mul(b,-1);
	c=add_sorted(a,b);
	printf("A-B=");
	print_terms(c);
	printf("%.15g\n",value(c,x));
	free_all(c);
	return 0;
}
