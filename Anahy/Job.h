#ifndef JOB_H
#define JOB_H

#include "definitions.h"
#include "JobId.h"
#include "JobAttributes.h"
#include <pthread.h>
#include <set>

using namespace std;

class VirtualProcessor;

enum JobState {ready, running, finished, blocked};

class Job {

	JobId id;
	Job* parent;
	set<Job*> children;
	VirtualProcessor* creator;
	JobState state;
	JobAttributes attributes;
	
	pfunc function;
	void* data;
	void* retval; // return value of 'function'
	
	pthread_mutex_t mutex;

	void add_child(Job* child); // called from the constructor
	Job(Job&); // to avoid copy construction
public:
	Job (JobId _id, Job* _parent, VirtualProcessor* _creator,
		JobAttributes _attributes, pfunc _function, void* _data);
	
	void run(); // to be called from a VP
	
	// atomic operations
	JobState compare_and_swap_state(JobState target_value, JobState new_value);  
	uint dec_join_counter();

	void display(int num_tabs=0);

	// getters and setters 
	JobId get_id() const;
	Job* get_parent() const;
	VirtualProcessor* get_creator() const;
	JobState get_state() const;
	JobAttributes get_attributes() const;
	void* get_retval() const;
	void set_retval(void* new_retval);
};

struct JobHandle {
	Job* pointer;
	JobId id;
};

#endif
