#include <pthread.h>
#include <queue>
#include <vector>

using namespace std;

class Work;
class Worker;
class WorkerEvent;

class Manager {

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t worker_event;
	
	int num_workers, workers_waiting;
	vector<Worker*> workers;
	queue<WorkerEvent*> event_queue, unhandled_events;
	queue<Work*> work_queue;

	static void* run_manager(void*);
	void run();
	void handle_event(WorkerEvent* event);
public:
	Manager(int workers);
	~Manager();

	// this messages are received from a worker,
	// like our VPs, so, they will be called from a
	// Worker thread!!!
	void request_work_and_wait(Worker* sender);
	void post_work(Work* work);

	// this messages are received from a control unit
	// like AnahyVM
	void start();
	void stop();
};