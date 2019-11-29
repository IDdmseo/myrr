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
	signal(SIGCHLD, sig_handler);
	/* variables */
	pid_t child, parent;
	struct rr_log *memory_log, *temp_memory_log;
	FILE *disk_log;
	int child_status, child_start = 0;

	/* monitor program logic */
	if (!strcmp(argv[1], "rc")) {
		init_all_information(RR_MOD_RECORD, RR_STARTED);
		parent = fork();
		if(parent == 0){
			child = getpid();
			execvp(argv[2], argv+2);
		}else if (parent != 0){
			// after SUT program finish and is terminated.
			waitpid(child, &child_status, 0);
			disk_log = fopen("rr_log.log", "wb");
			list_for_each_entry_safe(memory_log, temp_memory_log, &rr_log_head, list){
				fwrite(temp_memory_log, sizeof(struct rr_log), 1, disk_log);
				fwrite(temp_memory_log->copy_data, sizeof(char), temp_memory_log->data_size, disk_log);
				rr_remove_log(&rr_log_head);
			}
		}

		fclose(disk_log);
		init_all_information(RR_MOD_RECORD, RR_FINISHED);
	}

	else if(!strcmp(argv[1], "rp")) {
		init_all_information(RR_MOD_REPLAY, RR_STARTED);
		parent = fork();
		if(parent != 0){
			disk_log = fopen("rr_log.log", "rb");
			while(1){
				if(feof(disk_log))
					break;
				memory_log = (struct rr_log *)malloc(sizeof(struct rr_log));
				memset(memory_log, 0, sizeof(struct rr_log));
				fread(memory_log, sizeof(struct rr_log), 1, disk_log);
				if(memory_log->data_size > 0){
					memory_log->copy_data = (char *)malloc(memory_log->data_size);
					fread(memory_log->copy_data, sizeof(char), memory_log->data_size, disk_log);
					list_add_tail(&memory_log->list, &rr_log_head);
				}
			}
			fclose(disk_log);
		} else if(parent == 0){
			child = getpid();
			while(!child_start);
			execvp(argv[2], argv+2);
		}
		waitpid(child, &child_status, 0);
		init_all_information(RR_MOD_REPLAY, RR_FINISHED);
	}
}