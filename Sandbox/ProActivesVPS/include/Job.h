#ifndef JOB_H
#define JOB_H

#include "JobAttributes.h"
#include "JobId.h"

#include <pthread.h>
#include <set>

#include "definitions.h"

using namespace std;

class VirtualProcessor;

enum JobState {ready, running, finished};

class Job {

	JobId id;
	Job* parent;
	set<Job*> children;
	VirtualProcessor* creator;
	JobState state;
	JobAttributes* attributes;
		
	pfunc function;
	void* data;
	void* retval; // return value of 'function'
	
	Job(Job&); // to avoid copy construction
	
public:
	Job (JobId _id, Job* _parent, VirtualProcessor* _creator, JobAttributes* _attributes, pfunc _function, void* _data);
	~Job();

	void run(); // to be called from a VP
	
	// atomic operations
	bool compare_and_swap_state(JobState target_state, JobState new_state);
	
	//void display(int num_tabs=0);

	// getters and setters 
	inline JobId get_id() { return id; }
	inline Job* get_parent() const { return parent; }
	inline VirtualProcessor* get_creator() const { return creator; }
	inline void* get_retval() const { return retval; }
	inline JobAttributes* get_attributes() const { return attributes; }
	
	void set_retval(void* new_retval) { retval = new_retval; }
};

struct JobHandle {
	Job* pointer;
};

#endif
