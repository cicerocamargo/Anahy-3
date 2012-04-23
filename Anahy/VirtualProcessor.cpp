#include "VirtualProcessor.h"

/* STATIC MEMBERS' ITIALIZATIONS */
map<pthread_t,VirtualProcessor*> VirtualProcessor::vp_map;
uint VirtualProcessor::instance_counter = 0;

/* PUBLIC METHODS' DEFINITIONS */
VirtualProcessor::VirtualProcessor(Daemon* daemon) {
	this->daemon = daemon;
        id = instance_counter++;
}

VirtualProcessor::~VirtualProcessor() {
	
}

void VirtualProcessor::start() {
	
}

void VirtualProcessor::stop() {
	
}

void VirtualProcessor::flush() {
	
}

/* getters and setters */
Job* VirtualProcessor::get_current_job() {
	return current_job;
}

void VirtualProcessor::set_current_job(Job* new_value) {
	current_job = new_value;
}

uint VirtualProcessor::get_id() {
	return id;
}

ulong VirtualProcessor::get_job_counter() {
	return job_counter;
}

pthread_t VirtualProcessor::get_thread() {
	return thread;
}

pthread_mutex_t* VirtualProcessor::get_mutex() {
	return &mutex;
}

VirtualProcessor* VirtualProcessor::get_vp_from_pthraed(pthread_t thread_id)
const {
    return vp_map[thread_id];
}