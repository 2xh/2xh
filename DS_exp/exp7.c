#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX_SIZE 50
#define ERROR -1
struct MGraph {
	size_t size;
	char v[MAX_SIZE];
	int w[MAX_SIZE][MAX_SIZE];
};
size_t find(const struct MGraph* G,char x)
{
	for(size_t i=0;i<G->size;++i)
		if(G->v[i]==x)
			return i;
	return ERROR;
}
size_t degree(const struct MGraph* G,size_t pos)
{
	size_t degree=0;
	for(size_t i=0;i<G->size;++i)
		if(G->w[pos][i])
			++degree;
	return degree;
}
size_t dfs(const struct MGraph* G,size_t pos,void(*fun)(char))
{
	static bool path[MAX_SIZE];
	size_t step=0;
	if(G)
	{
		path[pos]=true;
		if(fun)
			fun(G->v[pos]);
		for(size_t i=0;i<G->size;++i)
			if(G->w[pos][i]&&!path[i])
				step+=dfs(G,i,fun);
		path[pos]=false;
		return ++step;
	}
}
struct List {
	size_t data;
	struct List* next;
};
struct List* new_node(size_t data)
{
	struct List *n=(struct List*)malloc(sizeof(struct List));
	if(!n)
		return NULL;
	n->data=data,n->next=NULL;
	return n;
}
struct List* shift(struct List* l)
{
	struct List *n=l->next;
	free(l);
	return n;
}
void bfs(const struct MGraph* G,size_t pos,void(*fun)(char))
{
	bool path[MAX_SIZE]={0};
	struct List *q=NULL,*r;
	if(G)
		r=q=new_node(pos),path[pos]=true;
	while(q)
	{
		pos=q->data;
		for(size_t i=0;i<G->size;++i)
			if(G->w[pos][i]&&!path[i])
				r->next=new_node(i),r=r->next,path[i]=true;
		if(fun)
			fun(G->v[pos]);
		q=shift(q);
	}
}
size_t gsort(const struct MGraph* G,void(*fun)(char))
{
	size_t indegree[MAX_SIZE]={0},i,count;
	int sum;
	struct List *q=NULL,*r;
	for(i=0;i<G->size;++i)
		for(size_t j=0;j<G->size;++j)
			if(G->w[i][j])
				++indegree[j];
	for(i=0;i<G->size;++i)
		if(!indegree[i])
		{
			if(q)
				r->next=new_node(i),r=r->next;
			else
				r=q=new_node(i);
		}
	for(sum=0,count=0;q;q=shift(q),count++)
	{
		i=q->data;
		for(size_t j=0;j<G->size;++j)
			if(G->w[i][j])
			{
				sum+=G->w[i][j];
				--indegree[j];
				if(!indegree[j])
					r->next=new_node(j),r=r->next;
			}
		if(fun)
			fun(G->v[i]);
	}
	if(!count)
		fputs("Loop detected, sort failed",stderr);
	return sum;
}
void prim(const struct MGraph* G,struct MGraph* MinTree)
{
	int dist[MAX_SIZE]={0};
	size_t pre[MAX_SIZE]={0};
	MinTree->size=G->size;
	for(size_t i=0,u=0,v=0;i<G->size&&v!=-1;++i)
	{
		v=-1;
		for(size_t j=0;j<G->size;++j)
		{
			if(pre[j]!=-1&&G->w[u][j]&&(!dist[j]||G->w[u][j]<dist[j]))
				dist[j]=G->w[u][j],pre[j]=u;
			if(dist[j]&&(v==-1||dist[j]<dist[v]))
				v=j;
		}
		if(v!=-1)
			MinTree->w[pre[v]][v]=G->w[pre[v]][v],MinTree->w[v][pre[v]]=G->w[v][pre[v]],
			pre[v]=-1,dist[v]=0;
		u=v;
	}
}
int main(void)
{
	struct MGraph G,MinTree;
	char c;
	size_t x;
	scanf("%zu",&G.size);
	for(size_t i=0;i<G.size;++i)
		scanf(" %c",G.v+i);
	for(size_t i=0;i<G.size;++i)
		for(size_t j=0;j<G.size;++j)
			scanf("%d",&G.w[i][j]);
	scanf(" %c",&c);
	if((x=find(&G,c))!=ERROR)
		printf("%zu\n",degree(&G,x));
	else
		printf("Not Found\n");
	printf("\n%zu\n",dfs(&G,x,(void(*)(char))putchar));
	bfs(&G,x,(void(*)(char))putchar);
	printf("\n");
	gsort(&G,(void(*)(char))putchar);
	printf("\n");
	prim(&G,&MinTree);
	for(size_t i=0;i<MinTree.size;++i)
	{
		for(size_t j=0;j<MinTree.size;++j)
			printf("%d ",MinTree.w[i][j]);
		printf("\n");
	}
	return 0;
}
/*test data 1:
5
a b c d e
0 6 2 0 0
6 0 3 4 3
2 3 0 1 0
0 4 1 0 5
0 3 0 5 0
a
*test data 2:
6
1 2 3 4 5 6
0 1 1 1 0 0
0 0 0 0 0 0
0 1 0 0 1 0
0 0 1 0 1 0
0 0 0 0 0 0
0 0 0 1 1 0
1
*/
