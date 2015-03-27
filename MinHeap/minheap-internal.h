/*
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Copyright (c) 2006 Maxim Yegorushkin <maxim.yegorushkin@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MINHEAP_INTERNAL_H_INCLUDED_
#define MINHEAP_INTERNAL_H_INCLUDED_

#include "event2/event-config.h"
#include "evconfig-private.h"
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/util.h"
#include "util-internal.h"
#include "mm-internal.h"

typedef struct min_heap
{
	struct event** p;	//二级指针，其中p[index]中存放的是 event *
	unsigned n, a;	//n队列元素的多少,a代表队列空间的大小
} min_heap_t;

static inline void	     min_heap_ctor_(min_heap_t* s);
static inline void	     min_heap_dtor_(min_heap_t* s);
static inline void	     min_heap_elem_init_(struct event* e);
static inline int	     min_heap_elt_is_top_(const struct event *e);
static inline int	     min_heap_empty_(min_heap_t* s);
static inline unsigned	     min_heap_size_(min_heap_t* s);
static inline struct event*  min_heap_top_(min_heap_t* s);
static inline int	     min_heap_reserve_(min_heap_t* s, unsigned n);
static inline int	     min_heap_push_(min_heap_t* s, struct event* e);
static inline struct event*  min_heap_pop_(min_heap_t* s);
static inline int	     min_heap_adjust_(min_heap_t *s, struct event* e);
static inline int	     min_heap_erase_(min_heap_t* s, struct event* e);
static inline void	     min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct event* e);
static inline void	     min_heap_shift_up_unconditional_(min_heap_t* s, unsigned hole_index, struct event* e);
static inline void	     min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct event* e);

#define min_heap_elem_greater(a, b) \
	(evutil_timercmp(&(a)->ev_timeout, &(b)->ev_timeout, >))

void min_heap_ctor_(min_heap_t* s) { s->p = 0; s->n = 0; s->a = 0; }
void min_heap_dtor_(min_heap_t* s) { if (s->p) mm_free(s->p); }
void min_heap_elem_init_(struct event* e) { e->ev_timeout_pos.min_heap_idx = -1; }
int min_heap_empty_(min_heap_t* s) { return 0u == s->n; }
unsigned min_heap_size_(min_heap_t* s) { return s->n; }
struct event* min_heap_top_(min_heap_t* s) { return s->n ? *s->p : 0; }

int min_heap_push_(min_heap_t* s, struct event* e)
{
	if (min_heap_reserve_(s, s->n + 1))	//判断空间是否足够
		return -1;
	min_heap_shift_up_(s, s->n++, e);	//找到队列中适合存放e的位置，并将其存入
	return 0;
}

struct event* min_heap_pop_(min_heap_t* s)
{
	if (s->n)
	{
		struct event* e = *s->p;	//取出队列首元素
		min_heap_shift_down_(s, 0u, s->p[--s->n]);	//对后面的n-1个元素重新调整
		e->ev_timeout_pos.min_heap_idx = -1;
		return e;
	}
	return 0;
}

int min_heap_elt_is_top_(const struct event *e)
{
	return e->ev_timeout_pos.min_heap_idx == 0;		//判断event是否是队列的第一个元素，即最小元素
}

int min_heap_erase_(min_heap_t* s, struct event* e)		//从队列中删除事件e
{
	if (-1 != e->ev_timeout_pos.min_heap_idx)	//事件e在队列中
	{
		struct event *last = s->p[--s->n];
		unsigned parent = (e->ev_timeout_pos.min_heap_idx - 1) / 2;		//e的父节点
		/* we replace e with the last element in the heap.  We might need to
		   shift it upward if it is less than its parent, or downward if it is
		   greater than one or both its children. Since the children are known
		   to be less than the parent, it can't need to shift both up and
		   down. */
		if (e->ev_timeout_pos.min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], last))	//不是队列首元素且父节点比最后一个元素大
			min_heap_shift_up_unconditional_(s, e->ev_timeout_pos.min_heap_idx, last);		//等价于将最后一个元素放到该位置，然后从该位置开始向上进行调整
		else
			min_heap_shift_down_(s, e->ev_timeout_pos.min_heap_idx, last);	//否则从e开始到最后一个元素进行调整
		e->ev_timeout_pos.min_heap_idx = -1;
		return 0;
	}
	return -1;
}

int min_heap_adjust_(min_heap_t *s, struct event *e)
{
	if (-1 == e->ev_timeout_pos.min_heap_idx) {
		return min_heap_push_(s, e);	//e不在队列中，则将其插入到队列中
	} else {
		unsigned parent = (e->ev_timeout_pos.min_heap_idx - 1) / 2;
		/* The position of e has changed; we shift it up or down
		 * as needed.  We can't need to do both. */
		if (e->ev_timeout_pos.min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], e))
			min_heap_shift_up_unconditional_(s, e->ev_timeout_pos.min_heap_idx, e);
		else
			min_heap_shift_down_(s, e->ev_timeout_pos.min_heap_idx, e);
		return 0;
	}
}

int min_heap_reserve_(min_heap_t* s, unsigned n)
{
	if (s->a < n)	//队列大小不足元素个数,重新分配空间
	{
		struct event** p;
		unsigned a = s->a ? s->a * 2 : 8;
		if (a < n)
			a = n;	//翻倍后空间依旧不足,则分配n.
		if (!(p = (struct event**)mm_realloc(s->p, a * sizeof *p)))		//重新分配内存
			return -1;
		s->p = p;	//重新赋值队列地址及大小
		s->a = a;
	}
	return 0;
}

void min_heap_shift_up_unconditional_(min_heap_t* s, unsigned hole_index, struct event* e)	//对从hole_index开始到e所在位置的所有元素进行调整
{
    unsigned parent = (hole_index - 1) / 2;		//开始元素的父节点
    do
    {
	(s->p[hole_index] = s->p[parent])->ev_timeout_pos.min_heap_idx = hole_index;	//不断向上调整
	hole_index = parent;
	parent = (hole_index - 1) / 2;
    } while (hole_index && min_heap_elem_greater(s->p[parent], e));
    (s->p[hole_index] = e)->ev_timeout_pos.min_heap_idx = hole_index;	//将e插入到该位置
}

void min_heap_shift_up_(min_heap_t* s, unsigned hole_index, struct event* e)
{
    unsigned parent = (hole_index - 1) / 2;		//从后往前调整，直到找到适合存放e的位置
    while (hole_index && min_heap_elem_greater(s->p[parent], e))
    {
	(s->p[hole_index] = s->p[parent])->ev_timeout_pos.min_heap_idx = hole_index;	//比父节点小或是到达根节点.则交换位置.循环.
	hole_index = parent;
	parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->ev_timeout_pos.min_heap_idx = hole_index;
}

void min_heap_shift_down_(min_heap_t* s, unsigned hole_index, struct event* e)	//从根节点开始向下查找最适合e的位置，e为最末尾节点，
{
    unsigned min_child = 2 * (hole_index + 1);	//2*i+2:为右孩子节点
    while (min_child <= s->n)
	{
		//找出较小子节点，min_child=min_child-1或min_child=min_child-0
	min_child -= min_child == s->n || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);	//min_child == s->n 表示已经是最后一个节点
	if (!(min_heap_elem_greater(e, s->p[min_child])))
	    break;
	(s->p[hole_index] = s->p[min_child])->ev_timeout_pos.min_heap_idx = hole_index;
	hole_index = min_child;
	min_child = 2 * (hole_index + 1);
	}
    (s->p[hole_index] = e)->ev_timeout_pos.min_heap_idx = hole_index;	//找到合适的位置将其插入
}

#endif /* MINHEAP_INTERNAL_H_INCLUDED_ */
