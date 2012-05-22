#include "Worker.h"
#include "Manager.h"
#include "ManagerController.h"
#include "Work.h"
#include <cstdio>
#include <cstdlib>

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
			say("going home.");
			break;
		}

		say("Working!");

		should_create_more_work = current->run();
		if (should_create_more_work) {
			manager->post_work(new Work());
		}
	}
}

Worker::Worker(Manager* m) : manager(m) {
	id = instances++;

	int i, num_tabs = id*2;
	tabs = (char*) malloc((num_tabs+1)*sizeof(char));
	for (i = 0; i < num_tabs; ++i) {
		tabs[i] = '\t';
	}
	tabs[i] = '\0';

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
	if (id == 0) {
		ManagerController::set_main_worker(this);
		puts("Woker 0 will run in main thread");
		return;
	}
	pthread_create(&thread, NULL, run_worker, this);
	printf("Starting Worker %d in a new thread (%lu).\n",
		id, (long) thread);
}

void Worker::stop() {
	pthread_join(thread, NULL);
}

void Worker::block() {
	pthread_mutex_lock(&mutex);
}

void Worker::say(const char* str) {
	//printf("%sW%d is %s\n", tabs, id, str);
}

// this is called from a Manager thread
void Worker::assign_work_and_resume(Work* w) {
	current = w;
	pthread_mutex_unlock(&mutex);	// unblock this worker's thread
}