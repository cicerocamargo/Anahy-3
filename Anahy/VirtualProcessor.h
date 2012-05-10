#ifndef VIRTUALPROCESSOR_H
#define VIRTUALPROCESSOR_H

#include <pthread.h>
#include <queue>
#include "definitions.h"
#include "JobId.h"
#include "Job.h"
#include "Daemon.h"


class VirtualProcessor {

	static uint instance_counter;	// tracks how many VP objects
									// have been created
	static pthread_key_t key;	// a key to store an VP object
								// in and retrieve it from a pthread
	
	uint id; // a unique id for this VP
	ulong job_counter; // the number of jobs created by this VP
	Job* current_job; // the job that is running at the moment

	queue<Job*> suspended_jobs; // to keep track of jobs blocked
								// on this VP
	
	Daemon* daemon; // object that handles scheduling operations
					// generated by this VP

	bool anahy_is_running; // indicates VMs state

	// variables to coordinate multithreaded execution 
	pthread_mutex_t mutex;
	pthread_cond_t operation_finished;

	// some private methods...
	void notify_finished_job_to_daemon(Job* job);
	void notify_new_job_to_daemon(Job* job);
	Job* ask_daemon_for_new_job(Job* job);

public:

	VirtualProcessor(Daemon* _daemon);
	virtual ~VirtualProcessor();

	// methods to initialize, retrieve and destroy the pthread_key
	static void init_pthread_key();
	static pthread_key_t get_pthread_key();
	static void delete_pthread_key();
	static void call_vp_destructor(void *vp_obj);

	// messages to be received from an AnahyVM object
	void start();
	void stop();
	void flush();
	
	// messages to be received from athread API
	JobId create_new_job(pfunc function, void* args, JobAttributes attr);
	void suspend_current_job_and_try_to_help(Job* job);
	void suspend_current_job_and_run_another(Job* job);

	// messages to be received from a Daemon
	void continue_execution();
	
	/* getters and setters */
	Job* get_current_job() const;
	void set_current_job(Job* new_value);
	uint get_id() const;
	ulong get_job_counter() const;
};

#endif