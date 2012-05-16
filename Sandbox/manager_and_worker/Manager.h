#include <pthread.h>
#include <queue>
#include <vector>

using namespace std;

class Work;
class Worker;
class WorkerEvent;

class Manager {
	static int instances;
	int id;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t event_cond;
	
	int num_workers;
	vector<Worker*> workers;
	queue<WorkerEvent*> event_queue, workers_waiting;
	
	static void* run_manager(void*);
	void run();
	void handle_event(WorkerEvent* event);
public:
	Manager(int workers);
	~Manager();

	// messages received from a worker,
	void request_work_and_wait(Worker* sender);
	void post_work(Work* work);

	// messages received from ManagerController
	void start();
	void stop();
};