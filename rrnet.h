#ifndef _RR_HEADER
#define _RR_HEADER

#define _GNU_SOURCE

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

/* RR_MOD */
#define RR_MOD_DEFAULT	0
#define RR_MOD_RECORD	1
#define RR_MOD_REPLAY	2

/* IO KINDS and SEQ */
#define SEND	3
#define RECV	4

/* log type */
#define RR_LOG_IO	5
#define RR_LOG_TID	6

/* definition of structure for Record-Replay */
struct list_head {
        struct list_head *next, *prev;
};

struct rr_log {
	int log_type;
	int rr_tid;
	int io_seq;
	int io_sort;
	int data_size;
	char *copy_data;	
	struct list_head list;
};

struct rr_tid {
	int rtid;
	int tid;
	struct list_head list;
}

/* log file configuration */
struct rr_log* rr_make_log(int type, char *data, int size, int sort, struct list_head *list);
struct rr_log* rr_get_log(struct list_head *list);
void rr_remove_log(struct list_head *list);
void init_all_information(int mode);
int rr_tid_alloc(int tid);
int find_rr_tid(int rtid);

/* global variables */
extern int curr_seq;
extern int curr_mode;
extern int rr_tid[2]; // 0 for tid, 1 for sleep
extern struct list_head rr_log_head;
extern struct list_head rr_tid_head;
extern pthread_cond_t cond[10];


/* Implementation of linked-list data structure */
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
struct list_head name = LIST_HEAD_INIT(name)
static inline void INIT_LIST_HEAD(struct list_head *list)
{
        list->next = list;
        list->prev = list;
}

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

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
        __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
        next->prev = prev;
        prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
        __list_del(entry->prev, entry->next);
        entry->next = (void *) 0;
        entry->prev = (void *) 0;
}

static inline void list_del_init(struct list_head *entry)
{
        __list_del(entry->prev, entry->next);
        INIT_LIST_HEAD(entry);
}

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

static inline int list_empty(struct list_head *head)
{
        return head->next == head;
}

#define list_entry(ptr, type, member) \
((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_first_entry(ptr, type, member)     \
        list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member)    \
        list_entry((pos)->member.next, type(*(pos)), member)

#define list_for_each_entry(pos, head, member)  \
        for (pos = list_first_entry(head, typeof(*pos), member);        \
                &pos->member != (head); \
                pos = list_next_entry(pos, member))
#endif
