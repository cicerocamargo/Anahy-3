#include <pthread.h>
#include <queue>
#include <vector>

using namespace std;

class Manager;
class Work;

class Worker {
	static int instances;
	int id;
	char* tabs;

	pthread_t thread;
	pthread_mutex_t mutex; // bin semaphore
	Manager* manager; // my
	Work* current;

	static void* run_worker(void* arg);
public:
	void run();

	Worker(Manager* m);
	~Worker();

	int get_id() { return id; }
	void start();
	void stop();
	void block();
	void say(const char* str);

	// called from a Manager thread
	void assign_work_and_resume(Work* w);
};