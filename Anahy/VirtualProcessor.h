#ifndef VIRTUALPROCESSOR_H
#define VIRTUALPROCESSOR_H

#include <pthread.h>
#include <map>
#include "definitions.h"
#include "JobId.h"
#include "Job.h"
#include "Daemon.h"

class VirtualProcessor {

	static map<pthread_t,VirtualProcessor*> vp_map;
	static uint instance_counter;
	
	uint id;
	ulong job_counter;
	Job* current_job;
	Daemon* daemon;
	pthread_t thread;

	bool program_end;
	pthread_mutex_t mutex;
	pthread_cond_t operation_finished;

	void notify_finished_job_to_daemon(Job* job);
	Job* ask_daemon_for_new_job(Job* job);

public:
	VirtualProcessor(Daemon* _daemon, pthread_t _thread); // default constructor
	virtual ~VirtualProcessor();

	void start();
	void stop();
	void flush();
	
	// messages to be received from athread API
	void notify_new_job(Job* job);
	void try_to_help_job(Job* job);
	void run_temp_job(Job* job);

	// messages to be received from a Daemon
	void signal_operation_finished();
	
	/* getters and setters */
	Job* get_current_job() const;
	void set_current_job(Job* new_value);
	uint get_id() const;
	ulong get_job_counter() const;
	pthread_t get_thread() const;
	pthread_mutex_t* get_mutex();
	JobId get_new_JobId();
	static VirtualProcessor* get_vp_from_pthread(pthread_t thread_id);
};

#endif