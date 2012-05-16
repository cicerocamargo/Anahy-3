#include "Manager.h"
#include "ManagerController.h"
#include "Work.h"
#include "Worker.h"
#include "WorkerEvent.h"
#include <cstdio>
#include <cstdlib>

int Manager::instances = 0;

void* Manager::run_manager(void* arg) {
	Manager* m = (Manager*) arg;
	m->run();
	return NULL;
}

void Manager::run() {
	WorkerEvent* event;
	Worker* worker;

	// start my workers
	for (int i = 0; i < num_workers; ++i) {
		workers[i]->start();
	}

	pthread_mutex_lock(&mutex);
	while (true) {
				
		if (workers_waiting.size() == num_workers) {
			// all workers are waiting, so this is the end ...
			while (workers_waiting.empty() == false) {
				event = workers_waiting.front(); // the related event
				workers_waiting.pop();
				worker = event->get_sender();
				worker->assign_work_and_resume(NULL);
				delete event;
			}
			pthread_mutex_unlock(&mutex);
			break;
		}
		else {
			if (event_queue.empty()) {
				printf("Manager %d: waiting for events...\n", id);
				pthread_cond_wait(&event_cond, &mutex);
			}
			else {
				while (event_queue.empty() == false) {
					event = event_queue.front();
					event_queue.pop();
					pthread_mutex_unlock(&mutex);
					handle_event(event);
					pthread_mutex_lock(&mutex);
				}
			}
		}
	}

	// stop my workers
	for (int i = 0; i < num_workers; ++i) {
		workers[i]->stop();
	} 
}

void Manager::handle_event(WorkerEvent* event) {
	Worker* worker;
	Work* work;
	WorkerEvent* unhandled;

	if (event->get_type() == PostWork) {
		printf("Manager %d: PostWork event received... ", id);
		if (workers_waiting.empty()) {
			ManagerController::post_work(event->get_work());
			puts("Work enqueued.");
			delete event;
		}
		else {
			unhandled = workers_waiting.front();
			workers_waiting.pop();
			worker = unhandled->get_sender();
			worker->assign_work_and_resume(event->get_work());
			printf("Bypassing Controller! Work assigned to Worker %d\n", worker->get_id());
			delete event;
			delete unhandled;
		}
	}
	else if (event->get_type() == GetWork) {
		printf("Manager %d: GetWork received from Worker %d... ",
			id, event->get_sender()->get_id());

		work = ManagerController::get_work();
		if (!work) {
			puts("No work. Worker is gonna wait.");
			workers_waiting.push(event);	
		}
		else {
			Worker* worker = event->get_sender();
			worker->assign_work_and_resume(work);
			puts("Work assigned!");
			delete event;				
		}
	}
	else {
		puts("Unknown event...");
		exit(1);
	}
}

Manager::Manager(int workers) : num_workers(workers) {
	id = instances++;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&event_cond, NULL);

	// create workers
	for (int i = 0; i < num_workers; ++i) {
		this->workers.push_back(new Worker(this));
	}
}

Manager::~Manager() {
	puts("Manager object deleted!");
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&event_cond);
	for (int i = 0; i < num_workers; ++i) {
		delete workers[i];
	}
	workers.clear();
}

// called from a worker thread
void Manager::request_work_and_wait(Worker* sender) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new WorkerEvent(GetWork, sender, NULL));
	pthread_cond_signal(&event_cond); // wake manager
	pthread_mutex_unlock(&mutex);
	sender->block(); // block worker thread
}

// called from a worker thread
void Manager::post_work(Work* work) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new WorkerEvent(PostWork, NULL, work));
	pthread_cond_signal(&event_cond);
	pthread_mutex_unlock(&mutex);
}

void Manager::start() {
	puts("Starting Manager...");
	// create my own thread
	pthread_create(&thread, NULL, run_manager, this); 
}

void Manager::stop() {
	// join my own thread
	pthread_join(thread, NULL);
	puts("Manager stopped!");
}