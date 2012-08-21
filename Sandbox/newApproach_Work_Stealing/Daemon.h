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

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	int num_vps, num_cpus;
	list<VirtualProcessor*> vps_waiting, vps_running;

	bool answer_oldest_vp_waiting();
	void broadcast_null();
	void take_vp_from_waiting_list(Job* vp);

public:

	Daemon(int _num_vps);
	~Daemon();

	void start_my_vps();
	void stop_my_vps();

	void put_vp_on_waiting_list(VirtualProcessor* vp);
	bool wake_up_some_waiting_vp();
	Job* work_stealing_function(VirtualProcessor* vp);

	inline int get_num_cpus() const { return num_cpus; }
};

#endif