#include <pthread.h>
#include <list>
#include "Job.h"

using namespace std;

class VP {
	pthread_t thread;
	pthread_mutex_t mutex; // this mutex protects my job_list
	list<Job*> job_list;
	Job* current_job;
public:
	VP();
	/* data */

	Job* get_local_job();
	void run();

	void resume() {
		pthread_mutex_unlock(&mutex);
	}

	void block() {
		pthread_mutex_lock(&mutex);
	}

	/* getters and setters*/
	void set_current_job(Job* job) {
		current_job = job;
	}
	Job* get_current_job() {
		return current_job;
	}
};