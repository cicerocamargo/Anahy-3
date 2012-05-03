#ifndef DAEMON_H
#define DAEMON_H

#include <queue>
#include <list>
#include <pthread.h>

using namespace std;

class AnahyVM;
class SchedulingOperation;

class Daemon {

	queue<SchedulingOperation*> new_operations;
	list<SchedulingOperation*> suspended_operations;
	pthread_mutex_t queue_mutex;
	AnahyVM* anahy;
	
public:
	Daemon(); // default constructor
	virtual ~Daemon();

	void start_and_run();
	void stop();
	void flush();
	void push_scheduling_operation(SchedulingOperation* sched_op);
	
	/* getters and setters */
	queue<SchedulingOperation*> get_new_operations() const;
	list<SchedulingOperation*> get_suspended_operations() const;
};

#endif