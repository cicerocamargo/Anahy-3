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
			// all workers are waiting
			printf("Manager %d: all workers waiting...\n", id);

			Work* work = ManagerController::blocking_get_work(this);

			if (!work) { // every manager was waiting and there was
						// no work, so STOP ALL MY WORKERS
				while (workers_waiting.empty() == false) {
					event = workers_waiting.front();
					workers_waiting.pop();
					worker = event->get_sender();
					worker->assign_work_and_resume(NULL);
					delete event;	
				}
				printf("Manager %d: bye!\n", id);
			}
			else { // satisfies the oldest GetJob
				event = workers_waiting.front();
				workers_waiting.pop();
				worker = event->get_sender();
				worker->assign_work_and_resume(work);
				delete event;
			}
			
			pthread_mutex_unlock(&mutex);
			break;
		}
		else {
			if (event_queue.empty()) {
				log << "Waiting for events...\n";
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
		log << "PostWork event received... ";
		printf("Manager %d: New work!!\n", id);
		if (workers_waiting.empty()) {
			ManagerController::post_work(event->get_work());
			log << "Work posted.";
			delete event;
		}
		else {
			unhandled = workers_waiting.front();
			workers_waiting.pop();
			worker = unhandled->get_sender();
			worker->assign_work_and_resume(event->get_work());
			log << "Bypassing Controller! Work assigned to Worker " << worker->get_id() << "\n";
			delete event;
			delete unhandled;
		}
	}
	else if (event->get_type() == GetWork) {
		log << "GetWork event received from Worker " << event->get_sender()->get_id() << " ... ";

		work = ManagerController::get_work();
		if (!work) {
			log << "No work. Worker is gonna wait.\n";
			event->get_sender()->say("waiting...");
			workers_waiting.push(event);	
		}
		else {
			Worker* worker = event->get_sender();
			worker->assign_work_and_resume(work);
			log << "Work assigned!\n";
			delete event;				
		}
	}
	else {
		log << "Unknown event...\n";
		exit(1);
	}
}

Manager::Manager(int workers) : num_workers(workers) {
	id = instances++;
	stop_signal = false;

	char file_name[20];
	sprintf(file_name, "logs/Manager%d.log", id);
	log.open(file_name, ifstream::out);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&event_cond, NULL);

	// create workers
	for (int i = 0; i < num_workers; ++i) {
		this->workers.push_back(new Worker(this));
	}
}

Manager::~Manager() {
	log << "Manager object deleted!\n";
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&event_cond);
	for (int i = 0; i < num_workers; ++i) {
		delete workers[i];
	}
	workers.clear();
	log.close();
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
	log << "Starting Manager...\n";
	// create my own thread
	pthread_create(&thread, NULL, run_manager, this); 
}

void Manager::signal_stop() {
	pthread_mutex_lock(&mutex);
	stop_signal = true;
	pthread_cond_signal(&event_cond);
	pthread_mutex_unlock(&mutex);
}

// called from ManagerController
void Manager::stop() {
	// join my own thread
	pthread_join(thread, NULL);
	log << "Manager stopped!\n";
}