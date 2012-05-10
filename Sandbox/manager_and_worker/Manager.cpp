#include "Manager.h"
#include "Work.h"
#include "Worker.h"
#include "WorkerEvent.h"
#include <cstdio>
#include <cstdlib>


void* Manager::run_manager(void* arg) {
	Manager* m = (Manager*) arg;
	m->run();
	return NULL;
}

void Manager::run() {
	WorkerEvent* event;

	while (true) {
		pthread_mutex_lock(&mutex);
		
		if (workers_waiting == num_workers && work_queue.empty()) {
			printf("All workers are waiting...");
			// if every worker is waiting no more work will be produced
			for (int i = 0; i < num_workers; ++i) {
				workers[i]->assign_work_and_resume(NULL); // to break worker loops
			}
			pthread_mutex_unlock(&mutex);
			break; // break the loop
		}
		else {
			if (event_queue.empty()) {
				puts("Manager waiting for worker events...");
				pthread_cond_wait(&worker_event, &mutex);
			}
			else {
				while (event_queue.empty() == false) {
					event = event_queue.front();
					event_queue.pop();
					handle_event(event);
				}
			}
			pthread_mutex_unlock(&mutex); // anyway
		}
	} 
}

void Manager::handle_event(WorkerEvent* event) {
	Worker* worker;
	Work* work;
	WorkerEvent* unhandled;

	if (event->get_type() == PostWork) {
		printf("PostWork event received... ");
		if (unhandled_events.empty()) {
			puts("Work enqueued.");
			work_queue.push(event->get_work());
			delete event;
		}
		else {
			unhandled = unhandled_events.front();
			unhandled_events.pop();
			worker = unhandled->get_sender();
			workers_waiting--;
			worker->assign_work_and_resume(event->get_work());
			printf("Work assigned to Worker %d\n", worker->get_id());
			delete event;
			delete unhandled;
		}
	}
	else if (event->get_type() == GetWork) {
		printf("GetWork event received from Worker %d... ", event->get_sender()->get_id());
		if (work_queue.empty()) {
			puts("Event enqueued.");
			unhandled_events.push(event);	
		}
		else {
			Worker* worker = event->get_sender();
			workers_waiting--;
			work = work_queue.front();
			work_queue.pop();
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
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&worker_event, NULL);

	// create workers
	for (int i = 0; i < num_workers; ++i) {
		this->workers.push_back(new Worker(this));
	}
	workers_waiting = 0;

	// create initial work
	for (int i = 0; i < 20; ++i) {
		work_queue.push(new Work());
	}
}

Manager::~Manager() {
	puts("Manager object deleted!");
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&worker_event);
	for (int i = 0; i < num_workers; ++i) {
		delete workers[i];
	}
	workers.clear();
}

// called from a worker thread
void Manager::request_work_and_wait(Worker* sender) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new WorkerEvent(GetWork, sender, NULL));
	workers_waiting++;
	pthread_cond_signal(&worker_event); // wake manager
	pthread_mutex_unlock(&mutex);
	sender->block(); // block worker thread
}

// called from a worker thread
void Manager::post_work(Work* work) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new WorkerEvent(PostWork, NULL, work));
	pthread_cond_signal(&worker_event);
	pthread_mutex_unlock(&mutex);
}

void Manager::start() {
	puts("Starting Manager...");

	// create my own thread
	pthread_create(&thread, NULL, run_manager, this); 

	// start my workers
	for (int i = 0; i < num_workers; ++i) {
		workers[i]->start();
	}
}

void Manager::stop() {
	// stop my workers
	for (int i = 0; i < num_workers; ++i) {
		workers[i]->stop();
	}
	
	// join my own thread
	pthread_join(thread, NULL);
	puts("Manager stopped!");
}