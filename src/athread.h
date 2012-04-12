#ifndef ATHREAD_H
#define ATHREAD_H

#include <pthread.h>

enum ThreadState {none, ready, running, finished, blocked};


typedef struct athread_attr_t {
        int stub;
};

class Job {
        
        static unsigned long counter;
        unsigned long id;
        void* (*pfunc)(void*);
        void* data;
        void* retval;
    
	Job* job;
	Job* parent;
	pthread_t* creator; 
	ThreadState st;
        athread_attr_t job_attributes;
	
public:
        Job();
	Job (void* (*func)(void*), void* job_data);
	//virtual ~athread_t ();

// getters and setters
	void set_job(Job* j);
	Job* get_job();
	void set_parent(athread_t* p);
	athread_t* get_parent();
	void set_creator(pthread_t* c);
	pthread_t* get_creator();
        unsigned long get_id();
        void run();
};

void aInit(int* _argc, char*** _argv);
void aTerminate();
int athread_create(athread_t* handle, athread_attr_t* attr,
	void*(*func)(void*), void* args);
int athread_join(athread_t handle, void** result);

#endif