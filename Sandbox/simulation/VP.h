#include <pthread.h>
#include <list>

using namespace std;

class VP {
	pthread_t thread;
	pthread_mutex_t mutex; // this mutex protects my job_list
	list<Job*> job_list;
	Job* current_job;
public:
	VP();
	
	/* data */
};