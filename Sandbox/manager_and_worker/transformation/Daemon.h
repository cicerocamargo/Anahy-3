#include <pthread.h>
#include <queue>
#include <vector>
#include <fstream>

using namespace std;

class Job;
class VirtualProcessor;

class Daemon {
	enum VPEventType { GetJob, NewJob };

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
	queue<VPEvent*> event_queue, vps_waiting;
	
	static void* run_daemon(void*);
	void run();
	void handle_event(VPEvent* event);

public:
	
	Daemon(int vps);
	~Daemon();

	// messages received from a vp
	void get_job(VirtualProcessor* sender);
	void new_job(Job* job);

	// messages received from AnahyVM
	inline int get_id() { return id; }
	void signal_stop();
	void start();
	void stop();
};