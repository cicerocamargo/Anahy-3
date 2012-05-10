#include "AnahyVM.h"
#include <cstdio>
#include <cstdlib>

/* STATIC MEMBERS' ITIALIZATIONS */
AnahyVM AnahyVM::unique_instance;

/* PUBLIC METHODS' DEFINITIONS */

AnahyVM::AnahyVM() {}

AnahyVM* AnahyVM::get_instance_handler() {
	return &unique_instance;
}

void* AnahyVM::run_daemon(void* daemon_obj) {
	Daemon* daemon = (Daemon*) daemon_obj;
	daemon->start();
	return NULL;
}

// messages to be received from the API

void AnahyVM::boot(uint _num_daemons = 1, uint _vps_per_daemon, uint _scheduling_function, uint mode_operation) {
	puts("Booting AnahyVM...");

	vps_per_daemon = _vps_per_daemon;
	num_daemons = _num_daemons;
	
	VirtualProcessor::init_pthread_key();
	
	// alloc arrays
	daemon_array = (Daemon**) malloc(num_daemons * sizeof(Daemon*));
	daemon_threads_array = (pthread_t*) malloc(num_daemons * sizeof(pthread_t));
	
	// create Daemons
	for (uint i = 0; i < num_daemons; ++i) {
		daemon_array[i] = new Daemon(vps_per_daemon);
	}

	// launch daemon threads
	for (uint i = 0; i < num_daemons; ++i) {
		pthread_create(&daemon_threads_array[i], NULL,
			run_daemon, (void*)&daemon_array[i]);
	}
	
	puts("Done!");
}

void AnahyVM::shut_down() {
	puts("Shuting AnahyVM down...");
	
	// join daemon threads
	for (int i = 0; i < num_daemons; i++) {
		pthread_join(daemon_threads_array[i], NULL);
	}
	
	// delete daemon objects
	for (uint i = 0; i < num_daemons; ++i) {
		delete daemon_array[i];
	}	
	
	// free arrays
	free(daemon_threads_array);
	free(daemon_array);
}

Job* AnahyVM::get_job_by_id(JobId job_id) {
	return job_map[job_id];
}

// MESSAGES TO BE RECEIVED FROM A DAEMON

// insert a new job in the graph
void AnahyVM::insert_job(Job* job) {
	// if this is a root job ...
	if (job->get_parent() == NULL) {
		root_jobs.push_back(job);
	}
	job_map[job->get_id()] = job;
}

// removes a job from the graph
void AnahyVM::remove_job(Job* job) {
	JobId id = job->get_id();

	// erase from the map
	job_map.erase(id); 

	// erase from the root list
	list<Job*>::iterator it;
	for(it = root_jobs.begin(); it != root_jobs.end(); it++) {
		if (id == (*it)->get_id()) {
			it = root_jobs.erase(it);
			break;
		}
	}
}

// uses the scheduling function to find
// the most suitable ready job in the graph
Job* AnahyVM::find_a_ready_job(Job* job) {
	return root_jobs.front();
	//return scheduling_function(root_jobs, job);
}


void AnahyVM::create_dummy_job(pfunc func, void* args) {
	JobId id(666,999);
	Job* parent = NULL;
	JobAttributes attr = 0;
	VirtualProcessor* vp = NULL;

	Job* j = new Job(id, parent, vp, attr, func, args);

	root_jobs.push_back(j);
	job_map[id] = j;
}