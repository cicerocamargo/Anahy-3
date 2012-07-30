#include "VP.h"

class Job;

Job* VP::get_local_job() {
	Job* j = 0;

	pthread_mutex_lock(&mutex);
	// removes the first job
	if (job_list.empty() == false) {
		j = job_list.front();
		job_list.pop_front();
	}	
	pthread_mutex_unlock(&mutex);
	return j;
}

void VP::run() {
	for (;;) {
		current_job = get_local_job();
		if (current_job == NULL) {
			__sync_add_and_fetch(&vps_waiting, 1);
			Daemon::find_a_ready_job(this); // this call blocks this thread
			__sync_sub_and_fetch(&vps_waiting, 1);
		}

		// if we got here, we have a job
		// or this could be the end of the program
		if (current_job == NULL) {
			break;
		}

		current_job->run();
		// TODO: how to unblock vps wating on this job???
	}	
}