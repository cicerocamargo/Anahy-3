#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <map>
#include <vector>
#include <list>

#include "definitions.h"
#include "VirtualProcessor.h"
#include "Job.h"
#include "JobId.h"
#include "Daemon.h"

class AnahyVM {
	
	uint num_processors;
	list<VirtualProcessor*> processors;
	Daemon* daemon;
	sfunc scheduling_function;
	list<Job*> root_jobs;
	map<JobId, Job*> job_map;

	pthread_t* vp_thread_array;
	
	static AnahyVM unique_instance;
	AnahyVM(); // default constructor
	AnahyVM(AnahyVM&); // copy-constructor

public:
	
	static AnahyVM* get_instance_handler();
	
	void boot(uint n_processors, sfunc scheduling_function);
	void shut_down();
	void insert_job(Job* job);
	void remove_job(Job* job);
	Job* find_a_ready_job(Job* job);
	
	/* getters and setters */
	list<Job*> get_root_jobs() const;
	map<JobId,Job*> get_job_map() const;
	
        int get_num_processors() const;
	vector<VirtualProcessor*> get_processors() const;
	Daemon* get_daemon() const;
	Job* get_job_by_id(JobId job_id);
	void set_scheduling_function(sfunc new_value);
};

#endif