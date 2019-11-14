#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>

int curr_seq;
int curr_mode;
int rr_tid[3]; // 0 for tid, 1 for sleep, 2 for total tid
struct list_head rr_log_head;
struct list_head rr_tid_head;
pthread_cond_t cond[10];
pthread_mutex_t sync_mutex;

struct rr_log* rr_make_log(int type, char *data, int size, int sort, struct list_head *list)
{
	int key = pthread_self();
	struct rr_log *log;
	
	log = (struct rr_log *)malloc(sizeof(struct rr_log));
	
	log->log_type = type;
	log->rr_tid = find_rr_tid(key);
	log->io_seq = curr_seq;
	log->io_sort = sort;
	log->data_size = size;
	
	log->copy_data = (char *)malloc(sizeof(char)*size);
	memcpy(log->copy_data, data, size);
	list_add_tail(&log->list, list);

	return log;
}

struct rr_log* rr_get_log(struct list_head *list)
{
	if(!list_empty(list))
		return list_first_entry(list, struct rr_log, list);

	else {
		printf("there are no logs\n");
		return NULL;
	}
}

void rr_remove_log(struct list_head *list)
{
	struct rr_log *log;
	
	if(list_empty(list)){
		printf("there are no logs\n");
		return;
	}
	
	log = rr_get_log(&rr_log_head);
	list_del(list->next);
	free(log->copy_data);
	free(log);	
}

void init_all_information(int mode)
{	
	int i;
	LIST_HEAD_INIT(&rr_log_head);
	LIST_HEAD_INIT(&rr_tid_head);
	curr_seq = 0;
	curr_mode = mode;

	for (i = 0;i < 2; i++)
		rr_tid[i] = 0;

	for (i = 0;i < 10; i++)
		cond[i] = PTHREAD_COND_INITIALIZER;
	pthread_mutex_init(&sync_mutex, NULL);
	
}
	

int rr_tid_alloc(int tid)
{
	struct rr_tid *rr_tid;
	rr_tid = (struct rr_tid *)malloc(sizeof(struct rr_tid));
	rr_tid->rtid = pthread_self(); // real thread id in pthread library.
	rr_tid->tid = rr_tid[0]; // alloc unique thread_id for rr.
	list_add_tail(&rr_tid->list, &rr_tid_head);

	rr_tid[0] += 1;

	return rr_tid->tid;
}

int find_rr_tid(int tid)
{
	struct rr_tid *get;
	get = list_first_entry(&rr_tid_list, struct rr_tid, list);

	if (get != NULL) {
		for (get; get->next != NULL; get=get->next) {
			if(tid != get->rtid)
				continue;
			else	
				return get->tid;
		}
	} else
		return NULL;
}


