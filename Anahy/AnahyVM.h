#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <map>
#include <vector>
#include <list>
#include <pthread.h>

#include "definitions.h"
#include "VirtualProcessor.h"
#include "Job.h"
#include "JobId.h"
#include "Daemon.h"

class AnahyVM {
	uint num_daemons, vps_per_daemon;
	
	// Daemon structures
	Daemon** daemon_array;
	pthread_t* daemon_threads_array;

	// function to be used in "Job* find_a_ready_job(Job* job)"
	sfunc scheduling_function;
	
	// graph variables
	list<Job*> root_jobs;
	map<JobId, Job*> job_map;

	static AnahyVM unique_instance;
	
	// starting routine to a daemon thread
	static void* run_daemon(void* daemon_obj);

	AnahyVM(); // default constructor
	AnahyVM(AnahyVM&); // copy-constructor

public:
	
	static AnahyVM* get_instance_handler();
	
	// messages to be received from the API
	void boot(uint _num_daemons, uint _vps_per_daemon, uint _scheduling_function, uint _mode_operation);
	void shut_down();
	Job* get_job_by_id(JobId job_id);

	// messages to be received from a Daemon
	void insert_job(Job* job);
	void remove_job(Job* job);
	Job* find_a_ready_job(Job* job);
	
	// for testing
	void create_dummy_job(pfunc func, void* args);
};

#endif
