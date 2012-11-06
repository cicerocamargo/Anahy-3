#include "include/Job.h"
#include "include/VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

Job::Job (JobId _id, Job* _parent, VirtualProcessor* _creator, JobAttributes* _attributes, pfunc _function, void* _data) :
	id(_id),
	parent(_parent),
	creator(_creator),
	attributes(_attributes),
	function(_function),
	data(_data)
{
	state = ready;
}

Job::~Job() {

}

void Job::run() {
    void* temp = (function)(data);
    retval = (temp ? temp : NULL);
    compare_and_swap_state(running, finished);
}

// the return value indicates operation's success (true) or failure
bool Job::compare_and_swap_state(JobState target_state, JobState new_state) {
	return __sync_bool_compare_and_swap (&state, target_state, new_state);
}

// auxiliary function
// void print_tabs(int num_tabs) {
// 	for (int i = 0; i < num_tabs; i++) {
// 		printf("\t");
// 	}
// }

// display job info
// void Job::display(int num_tabs) {
// 	print_tabs(num_tabs);
// 	printf("Job: ");
// 	id.display();
// 	printf("\n");
// 	if (parent) {
// 		print_tabs(num_tabs);
// 		printf("My Parent: ");
// 		(parent->get_id()).display();
// 		printf("\n");
// 	}
// 	print_tabs(num_tabs);
// 	printf("Creator: VP%u\n", creator->get_id());
// 	print_tabs(num_tabs);
// 	printf("State: %d\n", state);

// 	if (!children.empty()) {
// 		set<Job*>::iterator it;
// 		printf("My children:\n");
// 		for (it = children.begin(); it != children.end(); it++) {
// 			(*it)->display(num_tabs+1);
// 		}
// 	}
// }