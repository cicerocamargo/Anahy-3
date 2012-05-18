#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>


// private method
void Job::add_child(Job* child) {
	children.insert(child);
}

Job::Job (JobId _id, Job* _parent, VirtualProcessor* _creator, 
		JobAttributes* _attributes, pfunc _function, void* _data) :
	id(_id),
	parent(_parent),
	creator(_creator),
	attributes(_attributes),
	function(_function),
	data(_data)
{
    if(parent) {
        parent->add_child(this);
    }
	state = ready;
	pthread_mutex_init(&mutex, NULL);
}

void Job::run() {
    void* temp = (function)(data);
    retval = (temp ? temp : NULL);
}

// self explanatory ...
JobState Job::compare_and_swap_state(JobState target_value, JobState new_value) {
	pthread_mutex_lock(&mutex);

	JobState retval = state;
	if (state == target_value) {
		state = new_value;
	}

	pthread_mutex_unlock(&mutex);

	return retval;
}

// drecement atomically the number of joins that
// the job has to receive and return the value
uint Job::dec_join_counter() {
	pthread_mutex_lock(&mutex);

	// change attributes ???
	
	pthread_mutex_lock(&mutex);

	return 0;
}

// auxiliary function
void print_tabs(int num_tabs) {
	for (uint i = 0; i < num_tabs; i++) {
		printf("\t");
	}
}

void Job::display(int num_tabs) {
	print_tabs(num_tabs);
	printf("Job: ");
	id.display();
	printf("\n");
	if (parent) {
		print_tabs(num_tabs);
		printf("My Parent: ");
		(parent->get_id()).display();
		printf("\n");
	}
	print_tabs(num_tabs);
	printf("Creator: VP%u\n", creator->get_id());
	print_tabs(num_tabs);
	printf("State: %d\n", state);

	if (!children.empty()) {
		set<Job*>::iterator it;
		printf("My children:\n");
		for (it = children.begin(); it != children.end(); it++) {
			(*it)->display(num_tabs+1);
		}
	}
}

// getters and setters

void* Job::get_retval() const {
	return retval;
}

void Job::set_retval(void* new_retval) {
    retval = new_retval;
}

JobId Job::get_id() const {
    return id;
}

Job* Job::get_parent() const {
    return parent;
}

VirtualProcessor* Job::get_creator() const {
    return creator;
}

JobState Job::get_state() const {
    return state;
}

JobAttributes* Job::get_attributes() const {
	return attributes;
}