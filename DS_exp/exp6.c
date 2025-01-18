#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef char ElemType;
struct tnode {
    ElemType d;
    struct tnode *l,*r;
};
struct List {
    struct tnode* data;
    struct List* next;
};
struct List* new_node(struct tnode* data)
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
struct tnode* create_tree(char end)
{
    struct tnode *t=NULL;
    char c;
    if((c=getchar())!=EOF&&c!=end)
    {
        if(!(t=(struct tnode*)malloc(sizeof(struct tnode))))
        {
            perror(NULL);
            abort();
        }
        t->d=c;
        t->l=create_tree(end);
        t->r=create_tree(end);
    }
    return t;
}
void deltree(struct tnode* t)
{
    if(t)
        deltree(t->l),deltree(t->r),free(t);
}
size_t height(struct tnode *t)
{
    size_t l,r;
    return t?1+((l=height(t->l))>(r=height(t->r))?l:r):0;
}
void forfront(struct tnode *t,void(*fun)(ElemType))
{
    if(t)
        fun(t->d),forfront(t->l,fun),forfront(t->r,fun);
}
void formid(struct tnode *t,void(*fun)(ElemType))
{
    if(t)
        formid(t->l,fun),fun(t->d),formid(t->r,fun);
}
void forrear(struct tnode *t,void(*fun)(ElemType))
{
    if(t)
        forrear(t->l,fun),forrear(t->r,fun),fun(t->d);
}
void forlevel(struct tnode *t,void(*fun)(ElemType))
{
    struct List *q=NULL,*r;
    r=q=new_node(t);
    while(q)
    {
        t=q->data;
        if(t)
        {
            r->next=new_node(t->l),r=r->next;
            r->next=new_node(t->r),r=r->next;
            fun(t->d);
        }
        else
            fun(0);
        q=shift(q);
    }
}
void print(ElemType x)
{
    static size_t i=0,target=1;
    if(x)
        putchar(x);
    else
        putchar(' ');
    if(++i==target)
        printf("\n"),target<<=1,i=0;
}
int main(void)
{
    struct tnode* p=create_tree('#');
    forlevel(p,print);
    forfront(p,(void(*)(ElemType))putchar);
    putchar('\n');
    formid(p,(void(*)(ElemType))putchar);
    putchar('\n');
    forrear(p,(void(*)(ElemType))putchar);
    putchar('\n');
    forlevel(p,(void(*)(ElemType))putchar);
    putchar('\n');
    printf("%zu\n",height(p));
    deltree(p);
    return 0;
}
