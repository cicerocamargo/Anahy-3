#include "ManagerController.h"
#include "Manager.h"
#include "Work.h"

list<Manager*> ManagerController::managers;
queue<Work*> ManagerController::work_pool;
pthread_mutex_t ManagerController::mutex;

// messages from main

void ManagerController::init(int num_managers, int workers_per_manager) {
	for (int i = 0; i < num_managers; ++i) {
		// create managers
		managers.push_back(new Manager(workers_per_manager));
	}

	pthread_mutex_init(&mutex, NULL);

	// create initial work
	for (int i = 0; i < 10; ++i) {
		work_pool.push(new Work());
	}
}

void ManagerController::start() {
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		(*it)->start(); // start manager
	}
}

void ManagerController::stop() {
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		(*it)->stop(); // start manager
	}
}

void ManagerController::terminate() {
	list<Manager*>::iterator it;

	for (it = managers.begin(); it != managers.end(); ++it) {
		delete *it; // destroy manager
	}
	managers.clear();

	pthread_mutex_destroy(&mutex);
}

// messages from a Manager

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