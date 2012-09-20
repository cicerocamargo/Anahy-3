#ifndef JOB_H
#define JOB_H

#include "definitions.h"
#include "JobId.h"
#include <pthread.h>
#include <set>

using namespace std;

class VirtualProcessor;

enum JobState {ready, running, finished};

class Job {

	JobId id;
	Job* parent;
	set<Job*> children;
	VirtualProcessor* creator;
	VirtualProcessor* vp_thief;
	JobState state;
	
	pfunc function;
	void* data;
	void* retval; // return value of 'function'
	
	void add_child(Job* child); // called from the constructor
	Job(Job&); // to avoid copy construction
	
public:
	Job (JobId _id, Job* _parent, VirtualProcessor* _creator, pfunc _function, void* _data);
	~Job();

	void run(); // to be called from a VP
	
	// atomic operations
	bool compare_and_swap_state(JobState target_value, JobState new_value);  

	void remove_child(Job* child);

	void display(int num_tabs=0);

	// getters and setters 
	inline JobId get_id() { return id; }
	inline Job* get_parent() const { return parent; }
	inline set<Job*>& get_children() { return children; }
	inline VirtualProcessor* get_creator() const { return creator; }
	inline VirtualProcessor* get_vp_thief() const { return vp_thief; }
	inline JobState get_state() const { return state; }

	inline void* get_retval() const { return retval; }
	
	void set_vp_thief(VirtualProcessor* new_thief) { vp_thief = new_thief; }
	void set_retval(void* new_retval) { retval = new_retval; }
};

struct JobHandle {
	Job* pointer;
};

#endif
