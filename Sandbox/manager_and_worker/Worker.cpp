#include "Worker.h"
#include "Manager.h"
#include "Work.h"
#include <cstdio>

int Worker::instances = 0;

void* Worker::run_worker(void* arg) {
	Worker* w = (Worker*) arg;
	w->run();
	return NULL;
}

void Worker::run() {
	bool should_create_more_work = false;
	while (true) {
		manager->request_work_and_wait(this);
		
		if (!current) {
			break;
		}

		printf("VP %d is running a job with cost %d\n", id, current->amount);
		should_create_more_work = current->run();
		if (should_create_more_work) {
			printf("Worker %d: New Work!!!\n", id);
			manager->post_work(new Work());
		}
	}
}

Worker::Worker(Manager* m) : manager(m) {
	id = instances++;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	current = NULL;
}

Worker::~Worker()  {
	puts("Worker object deleted!");
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void Worker::start() {
	printf("Starting Worker %d in ", id);
	// if (id == 0) { // main worker
	// 	// set specific data
	// 	printf("main thread.\n")
	// 	return;
	// }
	// else {
	// 	pthread_create(&thread, NULL, run_worker, this);
	// 	printf("a new thread (%lu).\n", (long) thread);
	// }
	pthread_create(&thread, NULL, run_worker, this);
	printf("a new thread (%lu).\n", (long) thread);
}

void Worker::stop() {
	pthread_join(thread, NULL);
}

void Worker::block() {
	pthread_mutex_lock(&mutex);
}

// this is called from a Manager thread
void Worker::assign_work_and_resume(Work* w) {
	current = w;
	pthread_mutex_unlock(&mutex);	// unblock this worker's thread
}