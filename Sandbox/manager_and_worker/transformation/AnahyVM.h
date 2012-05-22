#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <list>
#include <queue>
#include <pthread.h>
#include "Job.h"
using namespace std;

class Daemon;
class VirtualProcessor;
class JobGraph;
class Job;

class AnahyVM {
	static uint num_daemons, daemons_waiting;
	static list<Daemon*> daemons;
	static JobGraph graph; // a graph of jobs

	static pthread_mutex_t mutex; 	// to control multiple
	static pthread_cond_t cond; 	// daemon threads

	static VirtualProcessor* main_vp; // the vp associated with main thread

	// constructors hidden to avoid instantiation
	AnahyVM();
	AnahyVM(AnahyVM&);
	~AnahyVM();

	static void start_vm();
	static void stop_vm();

public:
	// messages to be received from main
	static void init(int _num_daemons, int vps_per_daemon);
	static void terminate();
	static void create(JobHandle* handle, JobAttributes* attr,
		pfunc function, void* args);
	static void join(JobHandle handle, void** result);


	// messages to be received from a Daemon
	static Job* get_job(Job* joined_job);
	static Job* blocking_get_job();
	static void post_job(Job* new_job, bool scheduled);
	static void erase_job(Job* joined_job);
	static void set_main_vp(VirtualProcessor* vp);
};

#endif