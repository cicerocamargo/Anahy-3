#ifndef TASK_H
#define TASK_H

#include <set>
#include "serial_object.h"
#include <pthread.h>

class Scheduler;

class Task : public SerialObject {
protected:
	int pred_counter; // number of unsatisfied dependences
	pthread_mutex_t pred_counter_lock;
	std::set<Task*> successors, predecessors;
	std::set<Task*>::iterator it;
	void* result;
	
	friend class Scheduler;
	
	void add_predecessor(Task* t);
	
	// update the pred_counter of any successors of this
	// return value is a set of the successors which got ready
	std::set<Task*> update_successors();
	
	// atomically decrement pred_counter of this
	// if this task got ready (pred_counter == 0) insert it,
	// atomically, onto the caller's ready successors set
	void dec_pred_counter(std::set<Task*>& callers_ready_successors);

public:
	Task();
	virtual void run();
	void add_successor(Task* t);
	void* get_result();
};

#endif