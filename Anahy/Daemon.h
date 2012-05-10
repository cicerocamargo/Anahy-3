#ifndef DAEMON_H
#define DAEMON_H

#include "definitions.h"
#include <queue>
#include <list>
#include <pthread.h>

using namespace std;

class AnahyVM;
class SchedulingOperation;
class VirtualProcessor;

class Daemon {
	static uint instance_counter;	// tracks how many Daemon objects
									// have been created so far
	
	uint id; // a unique id for this Daemon

	// information about the processors
	// to be launched from this daemon
	uint num_processors;
	VirtualProcessor** vp_array;
	pthread_t* vp_threads_array;

	// structures to store and safely access
	// scheduling operations
	queue<SchedulingOperation*> pending_operations;
	pthread_mutex_t queue_mutex;
	pthread_cond_t new_sched_op;
	list<SchedulingOperation*> suspended_operations;
	
	AnahyVM* anahy; // a pointer to the VM object
	
	bool anahy_is_running; // indicates VMs state
	
	// some private methods
	static void* run_vp(void* vp_obj);
	void start_vps();

	// a method to evaluate a scheduling operation
	void execute_operation(SchedulingOperation* sched_op);

public:
	Daemon(uint _num_processors); // default constructor
	virtual ~Daemon();

	void start();
	void stop();
	void flush();

	// message to be received from a VP
	void push_scheduling_operation(SchedulingOperation* sched_op);
};

#endif