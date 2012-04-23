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
	Job* get_current_job();
	void set_current_job(Job* new_value);
	uint get_id();
	ulong get_job_counter();
	pthread_t get_thread();
	pthread_mutex_t* get_mutex();
};

#endif