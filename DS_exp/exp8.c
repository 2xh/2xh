#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
struct student
{
	unsigned long long num;
	char name[16];
	unsigned char score; /*0~255 enough for score*/
};
size_t find(const struct student* s,size_t len,unsigned long long num)
{
	for(size_t i=0;i<len;++i)
		if(s[i].num==num)
			return i;
	return -1;
}
size_t find2(const struct student* s,size_t len,unsigned long long num)
{
	int l=0,r=len-1,m;
	while(l<=r)
	{
		m=(l+r)/2;
		if(s[m].num==num)
			return m;
		else if(s[m].num<num)
			l=m+1;
		else
			r=m-1;
	}
	return -1;
}
void ssort(struct student* s,size_t len)
{
	struct student tmp;
	size_t min;
	for(size_t i=0;i<len;++i)
	{
		min=i;
		for(size_t j=i;j<len;++j)
			if(s[j].num<s[min].num)
				min=j;
		tmp=s[min],s[min]=s[i],s[i]=tmp;
	}
}
void isort(struct student* s,size_t len)
{
	struct student tmp;
	for(size_t i=1,j;i<len;++i)
	{
		tmp=s[i];
		for(j=i;j>0&&s[j-1].num>tmp.num;--j)
			s[j]=s[j-1];
		s[j]=tmp;
	}
}
void bsort(struct student* s,size_t len)
{
	struct student tmp;
	bool flag=false;
	for(size_t i=0;i<len&&!flag;++i)
	{
		flag=true;
		for(size_t j=len-1;j>i;--j)
			if(s[j].num<s[j-1].num)
			{
				tmp=s[j],s[j]=s[j-1],s[j-1]=tmp;
				flag=false;
			}
	}
}
void msort(const struct student* s,struct student* t,size_t len)
{
	if(len==1)
	{
		t[0]=s[0];
		return;
	}
	size_t half=len/2;
	msort(s,t,half),msort(s+half,t+half,len-half);
	struct student* tmp=(struct student*)malloc(sizeof(struct student)*len);
	if(!tmp)
		return;
	for(size_t i=0,p=0,q=half;i<len;++i)
		if(q<len&&(p>=half||t[p].num>t[q].num))
			tmp[i]=t[q++];
		else
			tmp[i]=t[p++];
	memcpy(t,tmp,sizeof(struct student)*len);
	free(tmp);
}
void sortq(struct student* s,size_t len)
{
	if(len<=1)
		return;
	unsigned long long ref=s[0].num;
	size_t i=0,j=len-1;
	struct student tmp;
	while(i<j)
	{
		while(i<j&&s[j].num>=ref)
			--j;
		while(i<j&&s[i].num<=ref)
			++i;
		tmp=s[i],s[i]=s[j],s[j]=tmp;
	}
	tmp=s[0],s[0]=s[i],s[i]=tmp;
	sortq(s,i);
	sortq(s+i+1,len-i-1);
}
struct tree
{
	struct student* data;
	struct tree* left;
	struct tree* right;
};
struct tree* create_BST(struct student* s,size_t len)
{
	struct tree* t=(struct tree*)malloc(sizeof(struct tree));
	struct tree* p,*q;
	t->data=s,t->left=t->right=NULL;
	for(size_t i=1;i<len;++i)
	{
		p=q=t;
		while(q)
		{
			if(s[i].num<q->data->num)
			{
				q=q->left;
				if(q)
					p=p->left;
			}
			else if(s[i].num>q->data->num)
			{
				q=q->right;
				if(q)
					p=p->right;
			}
			else break;
		}
		if(!q)
		{
			q=(struct tree*)malloc(sizeof(struct tree));
			q->data=s+i,q->left=q->right=NULL;
			s[i].num>p->data->num?(p->right=q):(p->left=q);
		}
	}
	return t;
}
struct student* findT(const struct tree *t,unsigned long long num)
{
	while(t)
	{
		if(num<t->data->num)
			t=t->left;
		else if(num>t->data->num)
			t=t->right;
		else return t->data;
	}
	return NULL;
}
enum method{SEL,INS,BUB,QUI,MER};
struct student* sort(const struct student* s,size_t len,enum method i)
{
	struct student* t=(struct student*)malloc(sizeof(struct student)*len);
	if(i!=MER)
		memcpy(t,s,sizeof(struct student)*len);
	switch(i)
	{
		case SEL:ssort(t,len);break;
		case INS:isort(t,len);break;
		case BUB:bsort(t,len);break;
		case QUI:sortq(t,len);break;
		case MER:msort(s,t,len);break;
	}
	return t;
}
void print(const struct student* s,size_t len)
{
	for(size_t i=0;i<len;++i)
		printf("%llu %s %hhu\n",s[i].num,s[i].name,s[i].score);
}
int main(void)
{
	struct student s[50],*t;
	size_t n,pos;
	unsigned long long num;
	char c=0;
	struct tree* bst;
	for(n=0;c!='\n'&&scanf("%llu %s %hhu%c",&(s+n)->num,(s+n)->name,&(s+n)->score,&c)>=3;++n);
	scanf("%llu",&num);
	pos=find(s,n,num);
	if(pos!=-1)
		print(s+pos,1);
	else
		printf("Not found\n");
	t=sort(s,n,SEL);
	print(t,n);
	free(t);
	printf("\n");
	t=sort(s,n,INS);
	print(t,n);
	free(t);
	printf("\n");
	t=sort(s,n,BUB);
	print(t,n);
	free(t);
	printf("\n");
	t=sort(s,n,QUI);
	print(t,n);
	free(t);
	printf("\n");
	t=sort(s,n,MER);
	print(t,n);
	pos=find2(t,n,num);
	if(pos!=-1)
		print(t+pos,1);
	else
		printf("Not found\n");
	free(t);
	bst=create_BST(s,n);
	t=findT(bst,num);
	if(t!=NULL)
		print(t,1);
	else
		printf("Not found\n");
	return 0;
}
