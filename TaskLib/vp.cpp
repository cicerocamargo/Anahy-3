#include "vp.h"
#include "scheduler.h"

bool VirtualProcessor::program_running = false;

VirtualProcessor::VirtualProcessor() {
	affinity_mask = 0;
	state = VPrunning;
}

VirtualProcessor::VirtualProcessor(unsigned long int aff_mask)
: affinity_mask(aff_mask) { }

VirtualProcessor::~VirtualProcessor() {
	
}

void VirtualProcessor::set_program_running(bool state) {
	program_running = state;
}

void VirtualProcessor::set_state(VPState state) {
	this->state = state;
}

VPState VirtualProcessor::get_state() {
	return state;
}

void VirtualProcessor::run() {
	Task* t = NULL;
	
	while (program_running) {
		t = Scheduler::task_request(this);
		
		if (t == NULL) {
			Scheduler::sleep_request(this);
		}
		else {
			t->run();
			Scheduler::graph_update_request(t);
		}
	}
	state = VPsleeping;
}