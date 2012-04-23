#ifndef VIRTUALPROCESSOR_H
#define VIRTUALPROCESSOR_H

#include <pthread.h>
#include <map>
#include "definitions.h"

class Job;
class Daemon;

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
	VirtualProcessor(Daemon* daemon); // default constructor
	virtual ~VirtualProcessor();

	void start();
	void stop();
	void flush();

	/* getters and setters */
	Job* get_current_job() const;
	void set_current_job(Job* new_value);
	uint get_id() const;
	ulong get_job_counter() const;
	pthread_t get_thread() const;
	pthread_mutex_t* get_mutex() const;
	JobId get_new_JobId();
   static VirtualProcessor* get_vp_from_pthread(pthread_t thread_id);
};

#endif