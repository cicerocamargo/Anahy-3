#ifndef JOB_H
#define JOB_H

#include "definitions.h"
#include "JobId.h"
#include <pthread.h>
#include <set>

using namespace std;

class VirtualProcessor;

typedef int JobAttributes;

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
	
public:
	Job (JobId _id, Job* _parent, VirtualProcessor* _creator,
		JobAttributes _attributes, pfunc _function, void* _data);
	
	void run();
	void add_child(Job* child);
	void display();
	
	// getters and setters
	JobId get_id() const;
	Job* get_parent() const;
	VirtualProcessor* get_creator() const;
	JobState get_state() const;
	JobAttributes get_attributes() const;
	void* get_retval() const;
};

#endif