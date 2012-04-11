#include "job.h"

unsigned long Job::counter = 0;

Job::Job(void* (*f)(void*), void* func_args) {
	func = f;
	args = func_args;
	id = counter++;
}	

unsigned long Job::get_id() {
	return id;
}

void Job::run() {
	(*func)(args);
}