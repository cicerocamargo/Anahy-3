#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <list>
#include <vector>
#include <fstream>

#include "Job.h"

using namespace std;

class VirtualProcessor;
class Daemon;
class Job;

class Daemon {
	
	static VirtualProcessor* main_vp;
	static list<VirtualProcessor*> vps;
	//static list<Job*> terminated_jobs; //we've got to discuss it
	static int num_vps, num_vps_waiting;
	int id;

	static pthread_mutex_t mutex;
	static pthread_cond_t cond;
		
	list<VirtualProcessor*> vps_waiting;
	
	void start_my_vps();
	void stop_my_vps();

	void answer_oldest_vp_waiting(Job* job);

	static void* run_daemon(void* arg);

	void run(); // main Daemon loop

	Daemon(int vps);
	Daemon(Daemon&);
	~Daemon();
	void star_my_vps();
	void stop_my_vps();

public:
	// this implements the user interface
	static void init(int _num_vps);
	static void terminate();
	static void create(JobHandle* handle, JobAttributes* attr,
		pfunc function, void* args);
	static void join(JobHandle handle, void** result);

	static void set_main_vp(VirtualProcessor* vp);
	inline void set_should_stop() { should_stop = true; }
	
};

#endif