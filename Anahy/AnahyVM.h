#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <map>
#include <vector>
#include "definitions.h"

class VirtualProcessor;
class Daemon;
class Job;
typedef ulong JobId;

class AnahyVM {
	
	uint num_processors;
	vector<VirtualProcessor*> processors;
	Daemon* daemon;
	sfunc scheduling_function;
	list<Job*> root_jobs;
	map<JobId, Job*> job_map;
	
	static AnahyVM unique_instance;
	AnahyVM(); // default constructor
	AnahyVM(AnahyVM&); // copy-constructor

public:
	
	static AnahyVM* get_instance_handler();
	
	void boot(uint n_processors, sfunc scheduling_function);
	void shut_down();
	void insert_job(Job*);
	void remove_job(Job*);
	Job* find_a_ready_job(Job*);
	
	/* getters and setters */
	list<Job*> get_root_jobs() const;
	map<JobId,Job*> get_job_map() const;
	int get_num_processors();
	vector<VirtualProcessor*> get_processors() const;
	Daemon* get_daemon();
	Job* get_job_by_id(JobId job_id) const;
	void set_scheduling_function(sfunc new_value);
};

#endif