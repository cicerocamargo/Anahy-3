#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <list>
#include <set>
#include "task.h"

class VirtualProcessor;

class Scheduler {
	// Initially, this list will contain the input nodes of the graph
	static std::list<Task*> ready_tasks;
	
	static int number_of_vps;
	static VirtualProcessor** vp_array;	// array of VP pointers
	static pthread_t* vp_thread_array;	// threads responsible for running the VPs
	
	// a monitor? a conditional variable?
	
	friend class VirtualProcessor;
	
	// just to prohibit the class to be instantiated from outside
	Scheduler();
	Scheduler(Scheduler&);
	
	// a request called from a VP asking for a task;
	// it can return NULL in case of no ready tasks available
	static Task* task_request();
		
	// a request called from a VP that just finished a task;
	// the scheduler should update evetual successor of the
	// "finished task"
	static void graph_update_request(Task* finished_task, pthread_mutex_t* mutex, pthread_cond_t* cond);

public:
	
		/* methods for the client programmer to set up the environment */
	
	// tells to the scheduler the number of VPs to be used
	// and and an optional set of input nodes
	static void init(int n_vps, std::list<Task*> input_nodes);
	
	static void terminate();
};

#endif