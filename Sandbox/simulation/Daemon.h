#include "VP.h"
#include "Job.h"
#include <list>

class Daemon {

public:
	/* data */
	list<VP*> running_list, waiting_list;

	pthread_cond_t cond;
	pthread_mutex_t mutex;

	void find_a_ready_job(VP* sender);
	void broadcast_null_job();
	void start_my_vps();
	void run();
};