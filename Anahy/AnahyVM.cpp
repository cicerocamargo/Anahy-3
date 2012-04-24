#include "AnahyVM.h"
#include <cstdio>

/* STATIC MEMBERS' ITIALIZATIONS */
AnahyVM AnahyVM::unique_instance;


/* PUBLIC METHODS' DEFINITIONS */

AnahyVM::AnahyVM() {
    
}

AnahyVM* AnahyVM::get_instance_handler() {
	return &unique_instance;
}

void AnahyVM::boot(uint n_processors, sfunc scheduling_function) {
	puts("Booting AnahyVM...");
	
	VirtualProcessor* vp = new VirtualProcessor(new Daemon());
	// do stuff
	
	puts("Done!");
}

void AnahyVM::shut_down() {
	puts("Shuting AnahyVM down...");
	
	// do stuff
}

void AnahyVM::insert_job(Job* job) {
	if (job->get_parent() == NULL) { // root job
		root_jobs.push_back(job);
	}
	job_map[job->get_id()] = job;
}

void AnahyVM::remove_job(Job* job) {
	
}

Job* AnahyVM::find_a_ready_job(Job* job) {
	
}

/* getters and setters */
list<Job*> AnahyVM::get_root_jobs() const {
	return root_jobs;
}

map<JobId,Job*> AnahyVM::get_job_map() const {
	return job_map;
}

int AnahyVM::get_num_processors() const {
	return num_processors;
}

vector<VirtualProcessor*> AnahyVM::get_processors() const {
	return processors;
}

Daemon* AnahyVM::get_daemon() const {
	return daemon;
}

Job* AnahyVM::get_job_by_id(JobId job_id) {
	return job_map[job_id];
}

void AnahyVM::set_scheduling_function(sfunc new_value) {
	scheduling_function = new_value;
}