#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <list>

using namespace std;

class Job;
class Daemon;
class VirtualProcessor;

class Daemon {

	pthread_t thread;

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	int num_vps, num_vps_waiting;
	list<VirtualProcessor*> vps_waiting, vps_running;
	
	void start_my_vps();
	void stop_my_vps();
	void broadcast_null_job();

	bool work_stealing_function(VirtualProcessor* vp);
	static void* run_daemon(void* arg);
	void run(); // main Daemon loop

public:

	Daemon(int _num_vps);
	~Daemon();

	//to be called from vps
	void waiting_for_a_job(VirtualProcessor* vp);
	
	void start();
	void stop();
};

#endif