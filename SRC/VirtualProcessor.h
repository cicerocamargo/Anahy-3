#ifndef VIRTUALPROCESSOR_H
#define VIRTUALPROCESSOR_H

#include <pthread.h>
#include <list>
#include "AnahyJobAttributes.h"
#include "lib/AnahySmartStack.h"
#include "lib/AnahySmartHeap.h"
#include "definitions.h"

using namespace std;

class AnahyJob;

class VirtualProcessor {
//instance vars
	int id;
	long job_counter;

	AnahySmartHeap<AnahyJob*> job_list;
	AnahyJob* current_job;
	AnahySmartStack<AnahyJob*> context_stack;

	pthread_mutex_t mutex; // protection to my job list
	pthread_attr_t attr;

// class vars
	static list<VirtualProcessor*> vp_list; // read-only access
	static int idle_vps; // atomic access!
	static int instance_counter;

	static void* call_vp_run(void *vp_obj);

//to set affinity
	int tid;
	static int tid_counter;

	void suspend_current_job_and_run_another();

public:

	VirtualProcessor();
	~VirtualProcessor();

	//getters
	inline int get_id() const { return id; }
	inline AnahyJob* get_current_job() const { return current_job; }
	inline int get_tid() const { return tid; }

	/* interface with the API */
	void run();
	void fork_job(AnahyJob* job);
	void* join_job(AnahyJob* job);


	AnahyJob* get_job();
	AnahyJob* steal_job();
};

#endif