#include "AnahyVM.h"
#include <cstdio>

/* STATIC MEMBERS' ITIALIZATIONS */
AnahyVM AnahyVM::unique_instance;


/* PUBLIC METHODS' DEFINITIONS */

AnahyVM* AnahyVM::get_instance_handler() {
	return &unique_instance;
}

void AnahyVM::boot(uint n_processors, sfunc scheduling_function) {
	puts("Booting AnahyVM...");
	
	// do stuff
	
	puts("Done!");
}

void AnahyVM::shut_down() {
	puts("Shuting AnahyVM down...");
	
	// do stuff
}

void AnahyVM::insert_job(Job* job) {
	
}

void AnahyVM::remove_job(Job* job) {
	
}

Job* AnahyVM::find_a_ready_job(Job* job) {
	
}

/* getters and setters */
list<Job*> AnahyVM::get_root_jobs() {
	return root_jobs;
}

map<JobId,Job*> AnahyVM::get_job_map() {
	return job_map;
}

int AnahyVM::get_num_processors() {
	return num_processors;
}

vector<VirtualProcessor*> AnahyVM::get_processors() {
	return processors;
}

Daemon* AnahyVM::get_daemon() {
	return daemon;
}

Job* AnahyVM::get_job_by_id(JobId job_id) {
	return job_map[job_id];
}

void AnahyVM::set_scheduling_function(sfunc new_value) {
	scheduling_function = new_value;
}

