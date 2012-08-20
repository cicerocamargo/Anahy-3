#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <list>
#include "JobGraph.h"

using namespace std;

class Job;
class Daemon;
class VirtualProcessor;

class Daemon {

	JobGraph* graph;

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	int num_vps, num_cpus;
	list<VirtualProcessor*> vps_waiting, vps_running;

	void answer_oldest_vp_waiting();
	void broadcast_null();

public:

	Daemon(int _num_vps);
	~Daemon();

	void start_my_vps();
	void stop_my_vps();

	//to be called from vps
	void post_job(Job* job);
	void request_job(Job* _starting_job, VirtualProcessor* vp);
	void erase_job(Job* joined_job, VirtualProcessor* vp);

	inline int get_num_cpus() const { return num_cpus; }

};

#endif