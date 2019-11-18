#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "rrnet.h"

int child_term = 0;
struct list_head rr_log_head;

void sig_handler(int signo){
	printf("SUT process terminated!\n");
	child_term += 1;
}

int main(int argc, char *argv[])
{
	pid_t child;
	int copy_data_size = 0;
	struct rr_tid *get;
	struct rr_log *log;
	FILE *fp_log;

	signal(SIGCHLD, sig_handler);
	
	if (argc != 3){
		perror("Usage: ./monitor rr_mode(rc or rp) ./executable_file\n");
		exit(1);
	}

	if(!strcmp(argv[1], "rc")) {
		init_all_information(RR_MOD_RECORD);
		fp_log = fopen("rr_log.log", "wb");
		child = fork();
		if (child == 0)
			execvp(argv[2], argv+2);
		else if (child != 0) {
			while(!child_term) {
				log = rr_get_log(&rr_log_head);
				fwrite(log, sizeof(int), 5, fp_log);
				fwrite(log->copy_data, sizeof(char), log->data_size, fp_log);
				rr_remove_log(&rr_log_head);
			}
		}
		fclose(fp_log);
		init_all_information(RR_MOD_DEFAULT);
		
	}

	else if(!strcmp(argv[1], "rp")) {
		init_all_information(RR_MOD_REPLAY);
		fp_log = fopen("rr_log.log", "rb");
		child = fork();
		if (child == 0)
			execvp(argv[2], argv+2);
		else if (child != 0) {
			while(!feof(fp_log)) {
				log = (struct rr_log *)malloc(sizeof(struct rr_log));
				fread(log, sizeof(int), 5, fp_log);
				fread(log->copy_data, sizeof(char), log->data_size, fp_log); 
				list_add_tail(&log->list, &rr_log_head);
			}
		}

		fclose(fp_log); 
		init_all_information(RR_MOD_DEFAULT);
	}

	/* remove all rr_tid_list */
	struct rr_tid *set, *next;

	list_for_each_entry_safe(set, next, &rr_tid_head, list){
		if (get == NULL)
			break;
		list_del(&get->list);
		free(get);
	}

	free(&rr_tid_head);

	return 0;
}	
