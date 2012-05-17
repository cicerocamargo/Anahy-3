#include "ManagerController.h"
#include "Manager.h"
#include "Worker.h"
#include "Work.h"

list<Manager*> ManagerController::managers;
queue<Work*> ManagerController::work_pool;
pthread_mutex_t ManagerController::mutex;
Worker* ManagerController::main_worker = 0;

// messages from main

void ManagerController::init(int num_managers, int workers_per_manager,
		int initial_work) {
	
	for (int i = 0; i < num_managers; ++i) {
		// create managers
		managers.push_back(new Manager(workers_per_manager));
	}

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);

	// create initial work
	for (int i = 0; i < initial_work; ++i) {
		work_pool.push(new Work());
	}

	start();
}

void ManagerController::start() {
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		(*it)->start(); // start manager
	}
	pthread_mutex_lock(&mutex);
	// wait for the main_worker to be set
	pthread_mutex_unlock(&mutex);
}

void ManagerController::stop() {
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		(*it)->signal_stop();
	}

	main_worker->run(); // execute remaining work ...

	for (it = managers.begin(); it != managers.end(); ++it) {
		(*it)->stop(); // start manager
		printf("Thread of manager %d joined!\n", (*it)->get_id());
	}
}

void ManagerController::terminate() {
	stop();
	
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		delete *it; // destroy manager
	}
	managers.clear();

	pthread_mutex_destroy(&mutex);
}

Worker* ManagerController::get_main_worker() {
	return main_worker;
}

// messages from a Manager

void ManagerController::set_main_worker(Worker* w) {
	// set main worker and unblock main thread
	// if it is waiting for the main worker to
	// be set
	main_worker = w;
	pthread_mutex_unlock(&mutex);
}

Work* ManagerController::get_work() {
	Work* w = NULL;
	pthread_mutex_lock(&mutex);

	if(work_pool.empty() == false) {
		w = work_pool.front();
		work_pool.pop();
	}

	pthread_mutex_unlock(&mutex);
	return w;
}

void ManagerController::post_work(Work* w) {
	pthread_mutex_lock(&mutex);

	work_pool.push(w);

	pthread_mutex_unlock(&mutex);

}