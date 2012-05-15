#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <map>
#include <list>
#include <pthread.h>

#include "definitions.h"
#include "VirtualProcessor.h"
#include "Job.h"
#include "JobId.h"
#include "Daemon.h"

class AnahyVM {
	uint num_daemons, vps_per_daemon;
	
	// Daemon list
	list<Daemon*> daemons;

	// function to be used in "Job* find_a_ready_job(Job* job)"
	sfunc scheduling_function;
	
	// graph variables
	list<Job*> root_jobs;
	map<JobId, Job*> job_map;

	static AnahyVM* unique_instance; // singleton

	// constructors and destructors are hidden
	// to avoid instantiation from outside
	AnahyVM(uint _num_daemons, uint _vps_per_daemon, uint _scheduling_function, uint _mode_operation);
	AnahyVM(AnahyVM&); // to avoid copy-construction
	~AnahyVM();
	
	void start(); // start daemons
	void stop(); // stop daemons

public:
	
	static AnahyVM* get_instance_handler();
	
	// messages to be received directly from the API
	static void boot(uint num_daemons, uint vps_per_daemon, uint scheduling_function, uint mode_operation);
	static void shut_down();

	// messages to be received from a Daemon
	void insert_job(Job* job);
	void remove_job(Job* job);
	Job* find_a_ready_job(Job* job);
	
	// for testing
	void create_dummy_job(pfunc func, void* args);
};

#endif
