#include <list>
#include <queue>
#include <pthread.h>
using namespace std;

class Manager;
class Worker;
class Work;

class ManagerController {
	static list<Manager*> managers;
	static queue<Work*> work_pool;
	static pthread_mutex_t mutex; // to allow multiple manager threads
	static Worker* main_worker;

	// constructors hidden to avoid instantiation
	ManagerController();
	ManagerController(ManagerController&);
	~ManagerController();

public:
	// messages to be received from main
	static void init(int num_managers, int workers_per_manager,
		int initial_work);
	static void start();
	static void start_main_worker();
	static void stop();
	static void terminate();
	static Worker* get_main_worker();

	// messages to be received from a Manager
	static void set_main_worker(Worker* w);
	static Work* get_work();
	static void post_work(Work* w);

	/* data */
};