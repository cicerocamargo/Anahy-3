#include <list>
#include <queue>
#include <pthread.h>
using namespace std;

class Manager;
class Work;

class ManagerController {
	static list<Manager*> managers;
	static queue<Work*> work_pool;
	static pthread_mutex_t mutex; // to allow multiple manager threads

	// constructors hidden to avoid instantiation
	ManagerController();
	ManagerController(ManagerController&);
	~ManagerController();

public:
	// messages to be received from main
	static void init(int num_managers, int workers_per_manager);
	static void start();
	static void stop();
	static void terminate();

	// messages to be received from a Manager
	static Work* get_work();
	static void post_work(Work* w);

	/* data */
};