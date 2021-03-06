#define _GNU_SOURCE
#include "rrnet.h"

int curr_seq;
int curr_mode;
int rr_tid_info[2]; // 0 for tid, 1 for sleep
struct list_head rr_log_head;
struct list_head rr_tid_head;
pthread_cond_t cond[10];
pthread_mutex_t sync_mutex;

LIST_HEAD(rr_log_head);
LIST_HEAD(rr_tid_head);

pthread_cond_t cond[10];
pthread_mutex_t sync_mutex;

struct rr_log* rr_make_log(int type, char *data, int size, int sort, struct list_head *list)
{
	printf("rr_make_log\n");
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

void init_all_information(int mode, int status)
{	
	struct rr_log *curr_del, *temp_del;
	int i = 0;

	if(status == RR_FINISHED){
		list_for_each_entry_safe(curr_del, temp_del, &rr_log_head, list){
			if(curr_del == NULL && temp_del == NULL)
				break;
			rr_remove_log(&rr_log_head);
		}
	}

	INIT_LIST_HEAD(&rr_log_head);
	INIT_LIST_HEAD(&rr_tid_head);
	curr_seq = 0;
	curr_mode = mode;

	for (i = 0;i < 2; i++){
		rr_tid_info[i] = 0; // 0 for total thread, 1 for sleep count
	}

	for (i = 0;i < 10; i++){
		pthread_cond_init(&cond[i], NULL);
	}
	pthread_mutex_init(&sync_mutex, NULL);	
}
	
int rr_tid_alloc(int curr_real_tid)
{
	struct rr_tid_log *tid_log;
	tid_log = (struct rr_tid_log *)malloc(sizeof(struct rr_tid_log));
	tid_log->real_tid = curr_real_tid; // real thread id in pthread library.
	tid_log->alloc_tid = rr_tid_info[0]; // alloc unique thread_id for rr.
	list_add_tail(&tid_log->list, &rr_tid_head);

	rr_tid_info[0] += 1;

	return tid_log->alloc_tid;
}

int find_rr_tid(int curr_real_tid)
{
	struct rr_tid_log *tid_log;
	list_for_each_entry(tid_log, &rr_tid_head, list) {
		if(curr_real_tid != tid_log->real_tid)
			continue;
		else if(tid_log == NULL)
			return -1;
		else
			return tid_log->alloc_tid;
	}
}