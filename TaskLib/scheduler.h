#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <list>
#include <set>
#include "task.h"

class VirtualProcessor;

class Scheduler {
	// Initially, this list will contain the input nodes of the graph
	static bool running;
	
	static std::list<Task*> ready_tasks;
	static pthread_mutex_t task_list_lock;
	static pthread_cond_t ready_task_cond;
	
	static int number_of_vps;
	static VirtualProcessor* vp_array;	// array of VPs
	static pthread_t* vp_thread_array;	// threads responsible for running the VPs
	
	// a monitor? a conditional variable?
	
	friend class VirtualProcessor;
	
	// just to prohibit the class to be instantiated from outside
	Scheduler();
	Scheduler(Scheduler&);
	
	// create pthreads to run the VPs
	static void create_vp_threads();
	
	// join the pthreads running the VPs
	static void join_vp_threads();

	// a request called from a VP asking for a task;
	// it can return NULL in case of no ready tasks available
	static Task* task_request(VirtualProcessor* vp);
	
	// a request called from a idle VP asking for sleep time;
	// the scheduler should put the VP waiting for a condition
	// variable to be true when there are ready tasks
	static void sleep_request(VirtualProcessor* vp);
	
	// a request called from a VP that just finished a task;
	// the scheduler should update evetual successor of the
	// "finished task"
	static void graph_update_request(Task* finished_task);

public:
	
		/* methods for the client programmer to set up the environment */
	
	// tells to the scheduler the number of VPs to be used
	// and and an optional set of input nodes
	static void init(int n_vps, std::list<Task*> input_nodes); 
	
	// adds a input node of the graph
	static void add_input_node(Task* t);
};

#endif