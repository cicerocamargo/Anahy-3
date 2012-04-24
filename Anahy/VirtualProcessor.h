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
	pthread_mutex_t mutex;

public:
	VirtualProcessor(Daemon* _daemon, pthread_t* _thread); // default constructor
	virtual ~VirtualProcessor();

	void start();
	void stop();
	void flush();

    void notify_new_job(Job* job);
    
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