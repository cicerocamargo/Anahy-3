#ifndef DAEMON_H
#define DAEMON_H

#include <pthread.h>
#include <queue>
#include <list>
#include <vector>
#include <fstream>

#include "Job.h"

using namespace std;

class VirtualProcessor;
class Daemon;
class Job;
class JobGraph;

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
	static JobGraph graph; //a graph of jobs
	static VirtualProcessor* main_vp;
	static list<VirtualProcessor*> vps;
	static int num_vps, vps_waiting;
	int id;

	fstream log;
	bool should_stop;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
		
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

	Daemon(int vps);
	Daemon(Daemon&);
	~Daemon();

	void start();
	void stop();

public:
	
	static void init(int _num_vps);
	static void terminate();
	static void create(JobHandle* handle, JobAttributes* attr,
		pfunc function, void* args);
	static void join(JobHandle handle, void** result);

	static Job* get_job(VPEvent event);
	static Job* blocking_get_job(Daemon* d);
	static void post_job(VPEvent event, bool scheduled);
	static void erase_job(Job* joined_job);
	static void forward_to_other_daemons(VPEvent event);
	static void forward_end_of_job(VPEvent event);
	static void set_main_vp(VirtualProcessor* vp);

	// messages received from a vp
	void get_job(VirtualProcessor* sender, Job* job);
	void new_job(VirtualProcessor* sender, Job* job);
	void end_of_job(VirtualProcessor* sender, Job* job);
	void destroy_job(VirtualProcessor* sender, Job* job);

	// messages received from AnahyVM
	void push_event(VPEvent event);
	inline int get_id() { return id; }
	inline void set_should_stop() { should_stop = true; }
	
};

#endif