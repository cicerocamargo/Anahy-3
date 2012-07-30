#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <list>

#include "Job.h"

using namespace std;

class VirtualProcessor;
class Daemon;
class Job;

class Daemon {
	//bool should_stop;
	static VirtualProcessor* main_vp;
		
	static int num_vps, num_vps_waiting;

	static pthread_mutex_t mutex;
	static pthread_cond_t cond;
		
	static list<VirtualProcessor*> vps_waiting, vps_running;

	static void answer_oldest_vp_waiting(Job* job);

	static void run(); // main Daemon loop

	Daemon();
	Daemon(Daemon&);
	~Daemon();

	static void start_my_vps();
	static void stop_my_vps();

public:
	// this implements the user interface
	static void init(int _num_vps);
	static void terminate();
	static void create(JobHandle* handle, JobAttributes* attr,
		pfunc function, void* args);
	static void join(JobHandle handle, void** result);

	//void broadcast_null_job();

	//to be called from vps
	static void waiting_for_a_job(VirtualProcessor* vp);
	static void remove_vp_from_waiting_list(VirtualProcessor* vp);

	static void set_main_vp(VirtualProcessor* vp);
	static void start();
};

#endif