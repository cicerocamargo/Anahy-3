#ifndef VIRTUALPROCESSOR_H
#define VIRTUALPROCESSOR_H

#include <pthread.h>
#include <stack>
#include <list>
#include "ourlist.h"
#include "definitions.h"

using namespace std;

class Job;
class JobHandle;
class JobAttributes;

class VirtualProcessor {
//instance vars
	int id;
	long job_counter;

	mList<Job*> job_list;
	Job* current_job;
	stack<Job*> context_stack;

	pthread_t thread; // my own thread
	pthread_mutex_t mutex; // protection to my job list
	pthread_attr_t attr;

	static pthread_key_t key;

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

	static void init_pthread_key();
	static void delete_pthread_key();
	static void call_vp_destructor(void* vp_obj);
	static void associate_vp_with_current_thread(void* vp_obj);

	//getters
	static VirtualProcessor* get_current_vp();
	inline int get_id() const { return id; }
	inline Job* get_current_job() const { return current_job; }

	/* interface with the API */
	JobHandle create_new_job(pfunc function, void* args);
	void* join_job(JobHandle handle);
	void run();

	Job* get_job();
	Job* steal_job();

	void start();
	void stop();
};

#endif