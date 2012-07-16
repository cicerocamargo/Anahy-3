#include <stdio.h>
#include <pthread.h>
#include <list>

int vps_waiting = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;

void create() {
	/* anyway */ create_new_local_job();

	if (__sync_val_compare_and_swap(&vps_waiting, vps_waiting, vps_waiting)) {
		// workarround to atommically discover the value of 'vps_waiting'
		// if we got here, vps_waiting > 0

		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}	
}

Job* get_local_job() {
	pthread_mutex_lock(&local_mutex); // this is shared with the daemon

	pthread_mutex_unlock(&local_mutex);
	return NULL;
}

void* vp(void* args) {
	for (;;) {
		Job* j = get_local_job();
		if (j == NULL) {
			__sync_add_and_fetch(&vps_waiting, 1);
			let_daemon_find_a_job(); // this call blocks this thread
			__sync_sub_and_fetch(&vps_waiting, 1);
		}

		// if we get here, we have a job
		// or this is the end of the program
		if (j == NULL) {
			break;
		}

		j->run();
		// TODO: how to unblock vps wating on this job???
	}
}

void* daemon(void* args) {
	pthread_mutex_lock()

}

int main(int argc, char** argv) {
	int num_vps = atoi(argv[1]);

}