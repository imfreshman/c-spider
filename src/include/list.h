#ifndef __LIST_H__
#define __LIST_H__

#include "common.h"

struct list_head{
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD(name) \
	struct list_head name = {&(name), &(name)};

//通过member获取结构首地址
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE*)0)->MEMBER)

#define container_of(ptr, type, member) ({	\
		const typeof( ((type*)0)->member ) *__mptr = (ptr);   \
		(type*)( (char *)__mptr - offsetof(type, member) );})

#define list_entry(ptr, type, member)	\
	container_of(ptr, type, member)

//
#define list_prev_entry(pos, member)	\
	list_entry( (pos)->member.prev, typeof(*(pos)), member)

#define list_first_entry(ptr, type, member)	\
	list_entry(ptr->next, type, member)

#define list_last_entry(ptr, type, member)	\
	list_entry(ptr->prev, type, member)

#define list_next_entry(pos, member)	\
	list_entry((pos)->member.next, typeof(*(pos)), member)



//链表遍历
#define list_for_each(pos, head) \
	for(pos = (head)->next; prefetch(pos->next), pos != (head); \
			pos = (pos)->next)

#define list_for_each_entry(pos, head, member)	\
	for(pos = list_entry( (head)->next, typeof(*pos), member);	\
		&pos->member != (head);					\
		pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)	\
	for(pos = list_entry( (head)->next, typeof(*pos), member),	\
		n = list_next_entry(pos, member);	\
		&pos->member != (head);		\
		pos = n, n = list_next_entry(n, member))

//链表逆向遍历
#define list_for_each_entry_reverse(pos, head, member)	\
	for(pos = list_entry((head)->prev, typeof(*(pos)), member);	\
		prefetch(pos->member.prev), &pos->member != (head);	\
		pos = list_prev_entry(pos, member))


//pos = list_last_entry(head, typeof(*(pos)), member)

#define list_for_each_entry_safe_reverse(pos, n, head, member)	\
	for(pos = list_entry((head)->prev, typeof(*(pos)), member),	\
		n = list_prev_entry(pos, member);	\
		&pos->member != (head);	\
		pos = n, n = list_prev_entry(n ,member))


/*
#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->prev = ptr; (ptr)->next = ptr; \
	}while(0)

*/

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->prev = list;
	list->next = list;
}

//双向链表头插法
static inline void __list_add(struct list_head *new,
		struct list_head *prev,
		struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

//双向链表尾插法
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}



//判断链表是否为空
static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

static	inline int list_empty_careful(const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}


//双向链表节点删除
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

//双向链表节点替换
static inline void list_replace(struct list_head *old, struct list_head *new)
{
	new->prev = old->prev;
	new->next = old->next;
	old->prev->next = new;
	old->next->prev = new;
}

static inline void list_replace_init(struct list_head *old, struct list_head *new)
{
	list_replace(old, new);
	INIT_LIST_HEAD(old);
}

//双向链接间的节点移动
static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

//判断是否是最后一个
static inline int list_is_last(const struct list_head *list,
		const struct list_head *head)
{
	return list->next == head;
}

static inline void __list_splice(const struct list_head *list,
			struct list_head *prev,
			struct list_head *next)
{

}

static inline void list_splice(const struct list_head *list,
		struct list_head *head)
{
	if(!list_empty(list))
		__list_splice(list, head, head->next);
}

static inline void list_splice_tail(struct list_head *list,
		struct list_head *head)
{
	if(!list_empty(list))
		__list_splice(list, head->prev, head);
}

static inline void list_splice_init(struct list_head *list,
		struct list_head *head)
{

}

static inline void list_splice_tail_init(struct list *list,
		struct list_head *head)
{

}

#endif
