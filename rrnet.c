#define _GNU_SOURCE
#include "rrnet.h"

int curr_seq;
int curr_mode;
int curr_rr_tid;
int rr_tid_info[2]; // 0 for tid and current total tid, 1 for sleep
struct list_head rr_log_head;
struct list_head rr_tid_head;

LIST_HEAD(rr_log_head);
LIST_HEAD(rr_tid_head);

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

struct rr_log* rr_get_log(struct list_head *plist)
{
	if(!list_empty(plist))
		return list_first_entry(plist, struct rr_log, list);

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
	
	log = rr_get_log(list);
	list_del(list->next);
	free(log->copy_data);
	free(log);	
}

void init_all_information(int mode)
{	
	int i = 0;

	INIT_LIST_HEAD(&rr_log_head);
	INIT_LIST_HEAD(&rr_tid_head);
	curr_seq = 0;
	curr_mode = mode;

	for (i = 0;i < 2; i++){
		rr_tid_info[i] = 0;
	}

	for (i = 0;i < 10; i++){
		pthread_cond_init(&cond[i], NULL);
	}
	pthread_mutex_init(&sync_mutex, NULL);
	
}
	

int rr_tid_alloc(int tid)
{
	struct rr_tid *get;
	get = (struct rr_tid *)malloc(sizeof(struct rr_tid));
	get->rtid = pthread_self(); // real thread id in pthread library.
	get->tid = rr_tid_info[0]; // alloc unique thread_id for rr.
	list_add_tail(&get->list, &rr_tid_head);

	rr_tid_info[0] += 1;

	return get->tid;
}

int find_rr_tid(int tid)
{
	struct rr_tid *get;

	list_for_each_entry(get, &rr_tid_head, list) {
		if(tid!=get->rtid)
			continue;
		else if(tid == get->rtid)
			return get->tid;
		else if(get == NULL);
			return 0;
	}
}

