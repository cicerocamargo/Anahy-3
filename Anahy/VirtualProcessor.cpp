#include "VirtualProcessor.h"
#include "Daemon.h"
#include "SchedulingOperation.h"
#include "definitions.h"

/* STATIC MEMBERS' ITIALIZATIONS */
map<pthread_t,VirtualProcessor*> VirtualProcessor::vp_map;
uint VirtualProcessor::instance_counter = 0;

/* PUBLIC METHODS' DEFINITIONS */
VirtualProcessor::VirtualProcessor(Daemon* _daemon) : daemon(_daemon) {
	pthread_t self = pthread_self();
	vp_map[self] = this;
	id = instance_counter++;
	job_counter = 0;
}

VirtualProcessor::~VirtualProcessor() {

}

void VirtualProcessor::start() {
	
}

void VirtualProcessor::stop() {
	
}

void VirtualProcessor::flush() {
	
}

void VirtualProcessor::notify_new_job(Job* job) {
    SchedulingOperation* op = new SchedulingOperation(NewJob, job, this);
    daemon->push_scheduling_operation(op);
}

/* getters and setters */
Job* VirtualProcessor::get_current_job() const {
	return current_job;
}

void VirtualProcessor::set_current_job(Job* new_value) {
	current_job = new_value;
}

uint VirtualProcessor::get_id() const {
	return id;
}

ulong VirtualProcessor::get_job_counter() const {
	return job_counter;
}

pthread_t VirtualProcessor::get_thread() const {
	return thread;
}

pthread_mutex_t* VirtualProcessor::get_mutex() {
	return &mutex;
}

JobId VirtualProcessor::get_new_JobId() {
	JobId jid(id, job_counter++);
	return jid;
}

VirtualProcessor* VirtualProcessor::get_vp_from_pthread(pthread_t thread_id) {
    return vp_map[thread_id];
}