#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

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
	//pthread_mutex_init(&mutex, NULL);
}

Job::~Job() {
	//delete attributes;
	// children.clear() e agora Mr M?
	if(parent)
		parent->remove_child(this);
}

void Job::run() {
    void* temp = (function)(data);
    retval = (temp ? temp : NULL);
    compare_and_swap_state(running, finished);
}

// the return value indicates operation's success (true) or failure
bool Job::compare_and_swap_state(JobState target_value, JobState new_value) {
	return __sync_bool_compare_and_swap (&state, target_value, new_value);
}

// drecement atomically the number of joins that
// the job is to receive and return true, if the counter
// reached ZERO
bool Job::dec_join_counter() {
	return attributes->dec_join_counter();
}

void Job::remove_child(Job* child) {
	//pthread_mutex_lock(&mutex);

	// IMPLEMENT!
	
	//pthread_mutex_unlock(&mutex);
}

// getters and setters

void* Job::get_retval() const {
	return retval;
}

void Job::set_retval(void* new_retval) {
    retval = new_retval;
}

JobId Job::get_id() {
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

// auxiliary function
void print_tabs(int num_tabs) {
	for (uint i = 0; i < num_tabs; i++) {
		printf("\t");
	}
}

// display job info
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