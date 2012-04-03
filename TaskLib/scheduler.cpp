#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "vp.h"
#include "check.h"

std::list<Task*> Scheduler::ready_tasks;
int Scheduler::number_of_vps = 0;
VirtualProcessor** Scheduler::vp_array;
pthread_t* Scheduler::vp_thread_array = NULL;


// PRIVATE METHODS

Scheduler::Scheduler() {}
Scheduler::Scheduler(Scheduler&) {}

void* run_vp(void* vp_object) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_object;
	vp->run();
	return NULL;
}

Task* Scheduler::task_request() {
	// this method should be called from a citical section
	Task* t = NULL;
	if (!ready_tasks.empty()) {
		t = ready_tasks.front();
		ready_tasks.pop_front();
	}
	return t;
}

void Scheduler::graph_update_request(Task* finished_task, pthread_mutex_t* mutex, pthread_cond_t* cond) {
	std::set<Task*> new_ready_tasks;
	std::set<Task*>::iterator it;
	int ret_code;
	
	// update successors of finished_task
	new_ready_tasks = finished_task->update_successors();
	
	// if any task got ready, push it onto the ready_tasks list
	if (!new_ready_tasks.empty()) {
		ret_code = pthread_mutex_lock(mutex);
		checkResults("pthread_mutex_lock", ret_code);
		
		for (it = new_ready_tasks.begin(); it != new_ready_tasks.end(); it++) {
			ready_tasks.push_back(*it);
		}
		
		if (VirtualProcessor::get_vps_waiting()) {
			// if there are vps waiting, signal them that new tasks arrived!
			ret_code = pthread_cond_broadcast(cond);
			checkResults("pthread_cond_broadcast", ret_code);
		}
		
		ret_code = pthread_mutex_unlock(mutex);
		checkResults("pthread_mutex_unlock", ret_code);
	}
}

// PUBLIC METHODS
void Scheduler::init(int n_vps, std::list<Task*> input_nodes) {
	number_of_vps = n_vps;
	vp_array = (VirtualProcessor**) malloc(number_of_vps * sizeof(VirtualProcessor*));
	vp_thread_array = (pthread_t*) malloc(number_of_vps * sizeof(pthread_t));
	for (int i = 0; i < number_of_vps; i++) {
		vp_array[i] = new VirtualProcessor();
	}
	ready_tasks = input_nodes;
	VirtualProcessor::set_program_running(true);
	
	// create a pthread to run each VP
	for (int i = 0; i < number_of_vps; i++) {
		pthread_create(&vp_thread_array[i], NULL, run_vp, (void*)vp_array[i]);
	}
}

void Scheduler::terminate() {
	for (int i = 0; i < number_of_vps; i++) {
		pthread_join(vp_thread_array[i], NULL);
	}
}