#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <queue>
#include <list>
#include <vector>
#include <fstream>

using namespace std;

class Job;
class Daemon;
class VirtualProcessor;

enum VPEventType { GetJob, NewJob, EndOfJob, DestroyJob,
		EndOfProgram };

class VPEvent {
	VPEventType type;
	VirtualProcessor* sender;
	Daemon* origin;
	Job* job;
	bool fwd;
	
public:
	inline VPEvent(VPEventType _type, VirtualProcessor* _sender,
			Daemon* _origin, Job* _job, bool _fwd=false)
	:	type(_type), sender(_sender),
		origin(_origin), job(_job), fwd(_fwd) {}

	inline VPEventType get_type() { return type; }
	inline VirtualProcessor* get_sender() { return sender; }
	inline Daemon* get_origin() { return origin; }
	inline Job* get_job() { return job; }
	inline bool get_fwd() { return fwd; }
	inline void set_fwd_true() { fwd = true; }
};


class Daemon {
	static int instances;
	int id;

	fstream log;
	bool should_stop;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t event_cond;
	
	int num_vps;
	vector<VirtualProcessor*> vps;
	queue<VPEvent> event_queue;
	list<VPEvent> vps_waiting;
	
	void start_my_vps();
	void stop_my_vps();
	void handle_event(VPEvent event);
	void handle_get_job(VPEvent event);
	void handle_new_job(VPEvent event);
	void handle_end_of_job(VPEvent event);
	
	void answer_oldest_vp_waiting(Job* job);

	static void* run_daemon(void* arg);

	void run(); // main Daemon loop

public:
	
	Daemon(int vps);
	~Daemon();

	// messages received from a vp
	void get_job(VirtualProcessor* sender, Job* job);
	void new_job(VirtualProcessor* sender, Job* job);
	void end_of_job(VirtualProcessor* sender, Job* job);
	void destroy_job(VirtualProcessor* sender, Job* job);

	// messages received from AnahyVM
	void push_event(VPEvent event);
	inline int get_id() { return id; }
	inline void set_should_stop() { should_stop = true; }
	void start();
	void stop();
};

#endif