#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
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
	/* ? */
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	int ret;

	if(curr_mode == RR_MOD_RECORD) {
		rr_make_log(RR_LOG_IO, buf, (int)len, SEND, rr_log_head);
		curr_seq += 1;
		return rr_send(sockfd, buf, len, flags);
	}

	else if(curr_mode == RR_MOD_REPLAY) {
retry:
		struct rr_log *log = rr_get_log(&rr_log_head);
		int curr_rr_tid = find_rr_tid(pthread_self());
		if (curr_rr_tid != log->rr_tid) {
			pthread_mutex_lock(&sync_mutex);
			pthread_cond_wait(&cond[curr_rr_tid], );
			pthread_mutex_unlock(&sync_mutex);
			goto retry;
		}
		else { 
			if (curr_seq != log->io_seq || log->io_sort != SEND) {
				if (rr_tid[1] == (rr_tid[0] - 1)) {
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_signal(&cond[log->tid]);
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					pthread_mutex_unlock(&sync_mutex);
					goto retry;
				}
				else {
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					pthread_mutex_unlock(&sync_mutex);
					goto retry;
				}
			}
			else if (curr_seq == log->io_seq && log->io_sort == SEND) {
				memcpy(buf, log->copy_data, len);	
				rr_remove_log(&rr_log_head);
				ret = rr_send(sockfd, buf, len, flags);
				curr_seq += 1;
				return ret;
			}
		}
	}
}

ssize_t recv(int sockfd, const void *buf, size_t len, int flags)
{
	int ret;

	if(curr_mode == RR_MOD_REPLAY) {
		rr_make_log(RR_LOG_IO, buf, (int)len, RECV, rr_log_head);
		curr_seq += 1;
		return rr_recv(sockfd, buf, len, flags);
	}

	else if (curr_mode == RR_MOD_REPLAY) {
retry:
		struct rr_log *log = rr_get_log(&rr_log_head);
		int curr_rr_tid = find_rr_tid(pthread_self());
		if (curr_rr_tid != log->rr_tid) {
			pthread_mutex_lock(&sync_mutex);
			pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
			pthread_mutex_unlock(&sync_mutex);
			goto retry;
		}
		else {
			if (curr_seq != log->io_seq || log->io_sort != RECV) {
				if (rr_tid[1] == (rr_tid[0] - 1)){
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_signal(&cond[log->rr_tid]);
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					pthread_mutex_unlock(&sync_mutex);
					goto retry;
				}
				else {
					pthread_mutex_lock(&sync_mutex);
					pthread_cond_wait(&cond[curr_rr_tid], &sync_mutex);
					pthread_mutex_unlock(&sync_mutex);
					goto retry;
				}
			}
			else {
				ret = rr_recv(sockfd, buf, len, flags);
				memcpy(buf, log->copy_data, len);
				rr_remove_log(&rr_log_head);
				curr_seq += 1;	
				return ret;
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
