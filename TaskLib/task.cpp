#include "task.h"

void Task::add_predecessor(Task* t) {
	predecessors.insert(t);
	pred_counter++;
}

std::set<Task*> Task::update_successors() {
	std::set<Task*>::iterator it;
	std::set<Task*> ready_successors;
	for (it = successors.begin(); it != successors.end(); it++) {
		(*it)->dec_pred_counter(ready_successors);
	}
	return ready_successors;
}

void Task::dec_pred_counter(std::set<Task*>& callers_ready_successors) {
	pthread_mutex_lock(&pred_counter_lock);
	/* CRITICAL SECTION */
	
	pred_counter--;
	if (pred_counter == 0) {
		callers_ready_successors.insert(this);
	}
	
	/* CRITICAL SECTION */
	pthread_mutex_unlock(&pred_counter_lock);
}

// PUBLIC METHODS

Task::Task() {
	pred_counter = 0;
	pthread_mutex_t pred_counter_lock = PTHREAD_MUTEX_INITIALIZER;
	result = NULL;
}

void Task::run() {}

void Task::add_successor(Task* t) {
	successors.insert(t);
	t->add_predecessor(this);
}

void* Task::get_result() {
	return result;
}