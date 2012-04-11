#include <stdio.h>
#include <pthread.h>
#include "athread.h"
#include "job.h"


athread_t::athread_t() {
	printf("New thread!\n");
}

void athread_t::set_job(Job* j) {
	job = j;
}

Job* athread_t::get_job() {
	return job;
}

void athread_t::set_parent(athread_t* p) {
	parent = p;
}

athread_t* athread_t::get_parent() {
	return parent;
}

void athread_t::set_creator(pthread_t* c) {
	creator = c;
}

pthread_t* athread_t::get_creator() {
	return creator;
}
