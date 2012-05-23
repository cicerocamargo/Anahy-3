#include <pthread.h>
#include <queue>
#include <list>
#include <vector>
#include <fstream>

using namespace std;

class Job;
class VirtualProcessor;

class Daemon {
	enum VPEventType { GetJob, NewJob, EndOfJob, DestroyJob };

	class VPEvent {
		VPEventType type;
		VirtualProcessor* sender;
		Job* job;
		
	public:
		inline VPEvent(VPEventType _type, VirtualProcessor* _sender, Job* _job) :
			type(_type), sender(_sender), job(_job) {}

		inline VPEventType get_type() { return type; }
		inline VirtualProcessor* get_sender() { return sender; }
		inline Job* get_job() { return job; }
	};

	static int instances;
	int id;

	fstream log;
	bool stop_signal;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t event_cond;
	
	int num_vps;
	vector<VirtualProcessor*> vps;
	queue<VPEvent*> event_queue;
	list<VPEvent*> vps_waiting;
	
	void start_my_vps();
	void stop_my_vps();
	void push_new_event(VPEvent* event);
	void handle_event(VPEvent* event);
	void handle_get_job(VPEvent* event);
	void handle_new_job(VPEvent* event);
	void handle_end_of_job(VPEvent* event);
	void handle_destroy_job(VPEvent* event);
	
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
	inline int get_id() { return id; }
	void start();
	void stop();
};