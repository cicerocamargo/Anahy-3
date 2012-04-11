#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <string.h>
#include "athread.h"
#include "job.h"
#include "graph_operation.h"

/* environment variables */

pthread_t main_thread, scheduler_daemon;
pthread_t* vp_array;
pthread_cond_t 	cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int num_vps = 1;
bool env_is_running;
std::list<athread_t*> root_threads;

/* environment variables */

Job* get_job(Job* starting_point) {
	int ret_code;
	
	/* STUB  */			return NULL; 		/* STUB  */
	
	if (!starting_point) {
		/* if no starting_point is defined 
			search for a job from the root */
	}
	else {
		
	}
}

void notify_completion(Job* job) {
	printf("Job %lu is done!", job->get_id());
}

void* scheduler_function(void* args) {
	int ret_code;
	pthread_t self = pthread_self();

	printf("Scheduler running on thread %lu!\n", (unsigned long) self);
	
	if (args) {
		// treat args ...
	}
	
	while (true) {
		ret_code = pthread_mutex_lock(&mutex);
		checkResults("pthread_mutex_lock", ret_code);
		
		if (!env_is_running) {
			ret_code = pthread_mutex_unlock(&mutex);
			checkResults("pthread_mutex_unlock", ret_code);
			break;
		}
		
		if (!graph_operations.empy()) {
			GraphOperation* operation = graph_operations.front();
			graph_operations.pop();
			ret_code = pthread_mutex_unlock(&mutex);
			checkResults("pthread_mutex_unlock", ret_code);
			operation->commit();
		}
		else {
			// wait for new graph operations...
		}
	}
}

void* vp_function(void* args) {
	Job* job;
	int ret_code;
	pthread_t self = pthread_self();

	printf("VP %lu running!\n", (unsigned long) self);
	
	while (true) {
		ret_code = pthread_mutex_lock(&mutex);
		
		if (!env_is_running) {
			ret_code = pthread_mutex_unlock(&mutex);
			break;
		}
		
		job = get_job(NULL);
		if (job) {
			ret_code = pthread_mutex_unlock(&mutex);
			job->run();
			notify_completion(job);
		}
		else { // go wait
			puts("waiting for new jobs or the end of the program...");
			ret_code = pthread_cond_wait(&cond, &mutex);
			// lock acquired... do something?
			ret_code = pthread_mutex_unlock(&mutex);
		}
	}
	
	printf("VP %lu shutting down...\n", (unsigned long) self);
	return NULL;
}

void aInit(int* _argc, char*** _argv) {
	main_thread = pthread_self();
	
	char** argv = *_argv;
	int argc = *_argc;
	
	// FIX: discover number of vps
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-vp") == 0 && (i+1) < argc) {
			num_vps = atoi(argv[i+1]);
		}
	}
	
	printf("Anahy started: %d VPs running.\n", num_vps);
	env_is_running = true;
	
	// allocate and launch VPs
	vp_array = (pthread_t*) malloc(num_vps*sizeof(pthread_t));
	for (int i = 0; i < num_vps; i++) {
		pthread_create(&vp_array[i], NULL, vp_function, NULL);
	}
}

void aTerminate() {
	printf("Shutting Anahy down...\n");
	int ret_code;
	ret_code = pthread_mutex_lock(&mutex);
	env_is_running = false;
	ret_code = pthread_cond_broadcast(&cond);
	ret_code = pthread_mutex_unlock(&mutex);
	
	for (int i = 0; i < num_vps; i++) {
		pthread_join(vp_array[i], NULL);
	}
}

int athread_create(athread_t* handle, athread_attr_t* attr, void*(*func)(void*), void* args) {
							
	pthread_t self = pthread_self();
	// handle->set_job(new Job(func, args));
	// handle->set_creator(self);
	// handle->set_parent(/* e agora?? */)

	// who am I ?
	if (self == main_thread) {
		// main thread

		printf("New thread created from main()\n");
	}
	else {
		// VP thread
		
		printf("New thread created by VP %lu\n", (unsigned long) self);
	}
}

int athread_join(athread_t handle, void** result) {
	//ThreadState st = handle->get_state();
	pthread_t self = pthread_self();
	
	// who am I ?
	if (self == main_thread) {
		// main thread

		printf("Thread joined from main()\n");
	}
	else {
		// VP thread
		
		printf("Thread joined by VP %lu\n", (unsigned long) self);
	}
}