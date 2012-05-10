#include <pthread.h>
#include <queue>
#include <vector>

using namespace std;

class Manager;
class Work;

class Worker {
	static int instances;
	int id;

	pthread_t thread;
	pthread_mutex_t mutex; // bin semaphore
	Manager* manager; // my
	Work* current;

	static void* run_worker(void* arg);
	void run();
public:

	Worker(Manager* m);
	~Worker();

	int get_id() { return id; }
	void start();
	void stop();
	void block();

	// called from a Manager thread
	void assign_work_and_resume(Work* w);
};