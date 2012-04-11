#ifndef ATHREAD_H
#define ATHREAD_H

#include <pthread.h>

class Job;

enum ThreadState {none, ready, running, finished, blocked};

class athread_t {
	Job* job;
	athread_t* parent;
	pthread_t* creator; 
	ThreadState st;
	
public:
	athread_t ();
	//virtual ~athread_t ();

// getters and setters
	void set_job(Job* j);
	Job* get_job();
	void set_parent(athread_t* p);
	athread_t* get_parent();
	void set_creator(pthread_t* c);
	pthread_t* get_creator();
};

typedef struct athread_attr_t {
	int stub;
};

void aInit(int* _argc, char*** _argv);
void aTerminate();
int athread_create(athread_t* handle, athread_attr_t* attr,
	void*(*func)(void*), void* args);
int athread_join(athread_t handle, void** result);

#endif