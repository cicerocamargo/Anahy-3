#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>

Job::Job (JobId _id, Job* _parent, VirtualProcessor* _creator,
	JobAttributes _attributes, pfunc _function, void* _data) :
id(_id),
parent(_parent),
creator(_creator),
attributes(_attributes),
function(_function),
data(_data) {
	state = ready;
}

void Job::run() {
    retval = (function)(data);
}

void Job::add_child(Job* child) {
	children.insert(child);
}

void Job::display() {
	id.display();
	if (parent) {
		printf("\tParent: ");
		(parent->get_id()).display();
	}
	printf("\nCreator: %ld\n", creator->get_id());
	printf("State: %d\n", state);
}

// getters and setters

void* Job::get_retval() const {
	return retval;
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

JobAttributes Job::get_attributes() const {
	return attributes;
}