#include "queue.h"
#include <iostream>
#include <assert.h>

using namespace std;

struct event_t{  //该结构体 中有很多变量，同时也有多个TAILQ，以其中的IO事件为例
	int val;		//将其它所有成员当做一个变量
	TAILQ_ENTRY(event_t) ev_next;	
};

TAILQ_HEAD(queue_head_t,event_t);

int main(){

	struct queue_head_t queue_head;
	struct event_t *p,*q,*s,*t;

	TAILQ_INIT(&queue_head);
	assert(queue_head.tqh_first==NULL);

	for(int i=0;i<3;++i){
		p=new event_t();
		p->val=i;
		TAILQ_INSERT_TAIL(&queue_head,p,ev_next);	//p在队列最尾部，三个元素分别为0,1,2
	}
	q=TAILQ_FIRST(&queue_head);	//队列首元素为0
	assert(TAILQ_END(&queue_head)==NULL);
	assert(q->val==0);
	assert(TAILQ_NEXT(q,ev_next)->val==1);
	assert(p==TAILQ_LAST(&queue_head,queue_head_t));	//p为队列尾部元素
	assert(TAILQ_PREV(p,queue_head_t,ev_next)->val==1);

	
	t=new event_t();
	t->val=7;
	TAILQ_INSERT_AFTER(&queue_head,q,t,ev_next);

	cout<<"foreach"<<endl;
	TAILQ_FOREACH(s,&queue_head,ev_next){
		cout<<s->val<<endl;
	}
	cout<<"foreach reverse"<<endl;
	TAILQ_FOREACH_REVERSE(s,&queue_head,queue_head_t,ev_next){
		cout<<s->val<<endl;
	}

	system("pause");
	return 0;
}