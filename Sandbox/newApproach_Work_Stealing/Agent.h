#ifndef AGENT_H
#define AGENT_H

#include <pthread.h>
#include <list>

using namespace std;

class Job;
class Agent;
class VirtualProcessor;

class Agent {

	pthread_mutex_t mutex;
	pthread_cond_t cond; //I don't know if this is no longer necessary

	int num_vps, num_cpus;
	list<VirtualProcessor*> request_list, vps_list;

	void broadcast_null();

public:

	Agent(int _num_vps);
	~Agent();

	void start_my_vps();
	void stop_my_vps();

	void put_vp_on_request_list(VirtualProcessor* vp);
	void attend_requests();

	inline int get_num_cpus() const { return num_cpus; }
};

#endif