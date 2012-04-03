#include "task.h"
#include "check.h"
#include <stdio.h>

std::set<Task*> Task::update_successors() {
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
		//printf("Task %lu got ready!!\n", get_id());
	}
	
	/* CRITICAL SECTION */
	pthread_mutex_unlock(&pred_counter_lock);
}

void Task::add_predecessor(Task* t) {
	predecessors.insert(t);
	pred_counter++;
}

// PUBLIC METHODS

void Task::add_successor(Task* t) {
	successors.insert(t);
	t->add_predecessor(this);
}

Task::Task() {
	pred_counter = 0;
//	pred_counter_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&pred_counter_lock, NULL);
	result = NULL;
}

void Task::run() {
	printf("No code implemented to task %ld...\n", get_id());
}

void* Task::get_result() {
	return result;
}
