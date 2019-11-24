#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <dlfcn.h>
#include "rrnet.h"

ssize_t (*rr_send)(int sockfd, const void *buf, size_t len, int flags) = NULL;
ssize_t (*rr_recv)(int sockfd, const void *buf, size_t len, int flags) = NULL;
int (*rr_pthread_create)(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) = NULL;

void __attribute__((constructor)) init_hooking()
{
	rr_send = dlsym(RTLD_NEXT, "send");
	rr_recv = dlsym(RTLD_NEXT, "recv");
	rr_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
}

void __attribute__((destructor)) finish_hooking()
{
	/* this part is just used for debugging library whether hooking was done well or not */
	fprintf(stdout, "hooking library call destructed\n");
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	int ret, curr_rr_tid;
	struct rr_log *log;

	if(curr_mode == RR_MOD_RECORD) {
		rr_make_log(RR_LOG_IO, buf, (int)len, SEND, &rr_log_head);
		curr_seq += 1;
		return rr_send(sockfd, buf, len, flags);
	}

	else if(curr_mode == RR_MOD_REPLAY){
	/*	
		1. 로그 파일을 읽어옴
		2. 로그 정보를 대조 (대조 기준: 오더링, 입출력 종류, 스레드 식별자)
			- matched ----> 실제 라이브러리 콜 호출
			- unmatched ----> 1. 현재 스레드를 제외한 모든 스레드가 sleep인가?
								Y: 로그에 명시된 순서의 스레드에 시그널을 보냄
								N: 현재 스레드를 sleep 상태로 전환	
	*/
wakeup_thread:
		log = rr_get_log(&rr_log_head);
		curr_rr_tid = find_rr_tid(pthread_self());

		if(curr_rr_tid != log->rr_tid){ // rr_tid unmatched
			if(rr_tid_info[1] != rr_tid_info[0] - 1) { // 슬립 스레드 != 전체 스레드 - 1
				pthread_mutex_lock(&sync_mutex);
				rr_tid_info[1] += 1;
				pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
				rr_tid_info[1] -= 1;
				pthread_mutex_unlock(&sync_mutex);
				goto wakeup_thread;
			} else { // 슬립 스레드 == 전체 스레드 - 1
				pthread_mutex_lock(&sync_mutex);
				pthread_cond_signal(&cond[log->rr_tid]);
				rr_tid_info[1] += 1;
				pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
				rr_tid_info[1] -= 1;
				pthread_mutex_unlock(&sync_mutex);
				goto wakeup_thread;		
			}
		} else { // rr_tid matched 
			if((curr_seq == log->io_seq) && (log->io_sort == SEND)){
				curr_seq += 1;
				ret =  rr_send(sockfd, log->copy_data, log->data_size, flags);
				rr_remove_log(&rr_log_head);
				return ret;
			} else {
				if(rr_tid_info[1] != rr_tid_info[0] - 1) { // 슬립 스레드 != 전체 스레드 - 1
					pthread_mutex_lock(&sync_mutex);
					rr_tid_info[1] += 1;
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					rr_tid_info[1] -= 1;
					pthread_mutex_unlock(&sync_mutex);
					goto wakeup_thread;
				} else {  // 슬립 스레드 == 전체 스레드 - 1
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_signal(&cond[log->rr_tid]);
					rr_tid_info[1] += 1;
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					rr_tid_info[1] -= 1;
					pthread_mutex_unlock(&sync_mutex);
					goto wakeup_thread;		
				}
			}
		}
	}
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
	int ret, curr_rr_tid;
	struct rr_log *log;

	if(curr_mode == RR_MOD_REPLAY) {
		rr_make_log(RR_LOG_IO, buf, (int)len, RECV, &rr_log_head);
		curr_seq += 1;
		return rr_recv(sockfd, buf, len, flags);
	}

	else if(curr_mode == RR_MOD_REPLAY){
	/*	
		1. 로그 파일을 읽어옴
		2. 로그 정보를 대조 (대조 기준: 오더링, 입출력 종류, 스레드 식별자)
			- matched ----> 실제 라이브러리 콜 호출
			- unmatched ----> 1. 현재 스레드를 제외한 모든 스레드가 sleep인가?
								Y: 로그에 명시된 순서의 스레드에 시그널을 보냄
								N: 현재 스레드를 sleep 상태로 전환	
	*/
wakeup_thread:
		log = rr_get_log(&rr_log_head);
		curr_rr_tid = find_rr_tid(pthread_self());

		if(curr_rr_tid != log->rr_tid){ // rr_tid unmatched
			if(rr_tid_info[1] != rr_tid_info[0] - 1) { // 슬립 스레드 != 전체 스레드 - 1
				pthread_mutex_lock(&sync_mutex);
				rr_tid_info[1] += 1;
				pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
				rr_tid_info[1] -= 1;
				pthread_mutex_unlock(&sync_mutex);
				goto wakeup_thread;
			} else { // 슬립 스레드 == 전체 스레드 - 1
				pthread_mutex_lock(&sync_mutex);
				pthread_cond_signal(&cond[log->rr_tid]);
				rr_tid_info[1] += 1;
				pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
				rr_tid_info[1] -= 1;
				pthread_mutex_unlock(&sync_mutex);
				goto wakeup_thread;		
			}
		} else { // rr_tid matched 
			if((curr_seq == log->io_seq) && (log->io_sort == RECV)){ // all things matched
				curr_seq += 1;
				ret =  rr_recv(sockfd, log->copy_data, log->data_size, flags);
				rr_remove_log(&rr_log_head);
				return ret;
			} else { // if not....
				if(rr_tid_info[1] != rr_tid_info[0] - 1) { // 슬립 스레드 != 전체 스레드 - 1
					pthread_mutex_lock(&sync_mutex);
					rr_tid_info[1] += 1;
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					rr_tid_info[1] -= 1;
					pthread_mutex_unlock(&sync_mutex);
					goto wakeup_thread;
				} else {  // 슬립 스레드 == 전체 스레드 - 1
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_signal(&cond[log->rr_tid]);
					rr_tid_info[1] += 1;
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					rr_tid_info[1] -= 1;
					pthread_mutex_unlock(&sync_mutex);
					goto wakeup_thread;		
				}
			}
		}
	}
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	int ret, rtid;
	ret = rr_pthread_create(thread, attr, start_routine, arg);

	if (curr_mode == RR_MOD_RECORD || curr_mode == RR_MOD_REPLAY) {
		rtid = pthread_self();
		rr_tid_alloc(rtid);
	}

	return ret;
}
