#ifndef _RR_HEADER
#define _RR_HEADER

#define _GNU_SOURCE

#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "list.h"

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
};

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
extern int curr_rr_tid;
extern int rr_tid_info[2]; // 0 for tid, 1 for sleep
extern struct list_head rr_log_head;
extern struct list_head rr_tid_head;
extern pthread_cond_t cond[10];
extern pthread_mutex_t sync_mutex;

#endif
