#ifndef DAEMON_H
#define DAEMON_H

#include <queue>
#include <list>
#include <pthread.h>

using namespace std;

class AnahyVM;
class SchedulingOperation;

class Daemon {

	queue<SchedulingOperation*> pending_operations;
	list<SchedulingOperation*> suspended_operations;
	pthread_mutex_t queue_mutex;
	AnahyVM* anahy;
	
	static void* run_vp(void* vp_obj);
	bool execute_operation(SchedulingOperation* sched_op);

public:
	Daemon(); // default constructor
	virtual ~Daemon();

	void start();
	void stop();
	void flush();
	void push_scheduling_operation(SchedulingOperation* sched_op);
};

#endif