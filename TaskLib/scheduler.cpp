#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "vp.h"



#define checkResults(string, val) {								\
	if (val) {															\
		printf("Failed with code %d at %s", val, string);	\
		exit(1);															\
	}																		\
}

bool Scheduler::running = false;
std::list<Task*> Scheduler::ready_tasks;
pthread_mutex_t Scheduler::task_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Scheduler::ready_task_cond;
int Scheduler::number_of_vps = 0;
VirtualProcessor* Scheduler::vp_array;
pthread_t* Scheduler::vp_thread_array = NULL;


// PRIVATE METHODS

Scheduler::Scheduler() {}
Scheduler::Scheduler(Scheduler&) {}

void* run_vp(void* vp_object) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_object;
	vp->run();
	return NULL;
}

void Scheduler::create_vp_threads() {
	// for any VP in the set	
	for (int i = 0; i < number_of_vps; i++) {
		// create a pthread to run the VP
		pthread_create(&vp_thread_array[i], NULL, run_vp, (void*)&vp_array);
	}
}

void Scheduler::join_vp_threads() {
	for (int i = 0; i < number_of_vps; i++) {
		pthread_join(vp_thread_array[i], NULL);
	}
} 

Task* Scheduler::task_request(VirtualProcessor* vp) {
	// acquire lock to manage the task list...
	Task* t = NULL;
	int ret_code;
	ret_code = pthread_mutex_lock(&task_list_lock);
   checkResults("pthread_mutex_lock()\n", ret_code);
	/* CRITICAL SECTION */
	
		// here comes the scheduling policy ?
	
	if (!ready_tasks.empty()) {
		t = ready_tasks.front();
		ready_tasks.pop_front();
	}
	
	/* CRITICAL SECTION */
	ret_code = pthread_mutex_unlock(&task_list_lock);
   checkResults("pthread_mutex_unlock()\n", ret_code);
	return t;
}

void Scheduler::sleep_request(VirtualProcessor* vp) {
	bool end_program = true;
	
	// a VP asked to sleep...	
	vp->set_state(VPsleeping);
	
	/* REVISAR ESTE IF-ELSE (LOCKS) */
	
	if (!ready_tasks.empty()) {
		end_program = false;
	}
	else {
		for (int i = 0; i < number_of_vps; i++) {
			if (vp_array[i].get_state() == VPrunning) {
				end_program = false;
				break;
			}
		}
	}
	
	if (end_program) {
		// if every VP is sleeping and there's no ready task
		VirtualProcessor::set_program_running(false);
		pthread_cond_signal(&ready_task_cond);
	}
	else {
		pthread_cond_wait(&ready_task_cond); //falta mutex
	}
	vp->set_state(VPrunning);
}

void Scheduler::graph_update_request(Task* finished_task) {
	std::set<Task*> new_ready_tasks;
	std::set<Task*>::iterator it;
	
	// update successors of finished_task
	new_ready_tasks = finished_task->update_successors();
	
	// if any got ready, push it onto the ready_tasks list
	int ret_code;
	ret_code = pthread_mutex_lock(&task_list_lock);
	checkResults("pthread_mutex_lock()\n", ret_code);
	/* CRITICAL SECTION */
	
	for (it = new_ready_tasks.begin(); it != new_ready_tasks.end(); it++) {
		ready_tasks.push_back(*it);
	}
	
	/* CRITICAL SECTION */
	ret_code = pthread_mutex_unlock(&task_list_lock);
	checkResults("pthread_mutex_unlock()\n", ret_code);
	
	if (!ready_tasks.empty()) {
		pthread_cond_signal(&ready_task_cond);
	}
}

// PUBLIC METHODS
void Scheduler::init(int n_vps, std::list<Task*> input_nodes) {
	number_of_vps = n_vps;
	vp_array = (VirtualProcessor*) malloc(number_of_vps * sizeof(VirtualProcessor));
	vp_thread_array = (pthread_t*) malloc(number_of_vps * sizeof(pthread_t));
	for (int i = 0; i < number_of_vps; i++) {
		vp_array[i] = *(new VirtualProcessor());
	}
	ready_tasks = input_nodes;
	running = true;
	VirtualProcessor::set_program_running(true);
	pthread_cond_init(&ready_task_cond, NULL);
	Scheduler::create_vp_threads();
}

void Scheduler::add_input_node(Task* t) {
	if (running) {
		fprintf(stderr,"Program already running... impossible to add entry nodes to the graph!\n");
	}
	else {
		ready_tasks.push_back(t);
	}
}