#ifndef ATHREAD_H
#define ATHREAD_H

#include <pthread.h>

enum ThreadState {none, ready, running, finished, blocked};

//We need to define the thread's attributes, but not at this moment
typedef struct athread_attr_t {
        int stub;
};

//I think that define pfunc here it's better to use it
typedef void *(*pfunc)(void *);

class Job {
    
        static unsigned long counter;
        unsigned long id;
        pfunc function;
        void* data;
        void* retval;
    
	Job* job;
	Job* parent;
	Job* creator;
	ThreadState state;
        athread_attr_t job_attributes;
	
public:
        Job();
	Job (pfunc func, void* job_data);
	//virtual ~athread_t ();

// getters and setters
	void set_job(Job* j);
	Job* get_job() const;
	void set_parent(athread_t* p);
	athread_t* get_parent() const;
	void set_creator(pthread_t* c);
	pthread_t* get_creator() const;
        ThreadState get_state() const;
        unsigned long get_id() const;
        void run();
};

void aInit(int* _argc, char*** _argv);
void aTerminate();
int athread_create(Job* handle, athread_attr_t* attr,
	pfunc function, void* args);
int athread_join(Job* handle, void** result);

#endif