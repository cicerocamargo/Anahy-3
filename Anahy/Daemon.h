#ifndef DAEMON_H
#define DAEMON_H

#include "definitions.h"
#include <queue>
#include <list>
#include <pthread.h>

using namespace std;

class AnahyVM;
class VPEvent;
class VirtualProcessor;

class Daemon {
	static uint instance_counter;	// tracks how many Daemon objects
									// have been created so far
	
	uint id; // a unique id for this Daemon
	pthread_t thread; // the thread that will run this daemon

	uint num_processors; // number of processors to be launched from this daemon
	uint processors_waiting;
	list<VirtualProcessor*> processors;

	// structures to store and
	// safely handle VP events
	queue<VPEvent*> event_queue;
	list<VPEvent*> suspended_events;
	pthread_mutex_t queue_mutex;
	pthread_cond_t event_condition;
	
	
	AnahyVM* anahy; // a pointer to the VM object
	
	bool anahy_stop; 	// signal that AnahyVM changes
						// when daemon should to stop
	
	// some private methods
	static void* call_daemon_run(void* daemon_obj);
	void run();
	void push_vp_event(VPEvent* event);
	void handle_vp_event(VPEvent* sched_op);

public:
	Daemon(uint _num_processors); // default constructor
	virtual ~Daemon();

	// messages to be received from anahy
	void start();
	void stop();

	// messages to be received from a VP
	void new_job(VirtualProcessor* sender, Job* job);	// sender created Job 'job', and I
														// should insert in Anahy's graph
	void get_job(VirtualProcessor* sender, Job* job);	// sender wants a Ready Job that 
														// should be searched apart from 'job'
	void end_of_job(VirtualProcessor* sender, Job* job); // sender finished Job 'job'
	void destroy_job(VirtualProcessor* sender, Job* job);	// sender did the last join on
															// Job 'job'
};

#endif