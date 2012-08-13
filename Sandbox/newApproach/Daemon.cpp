#include "Daemon.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

/**** PRIVATE METHODS ****/

/* This method is too large and "ugly?"*/

Daemon::Daemon(int _num_vps) : num_vps(_num_vps) {
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	graph = new JobGraph();

	printf("DAEMON: Creating vps\n");
	//create my vps
	for(int i = 0; i < num_vps; i++) {
		vps_running.push_back(new VirtualProcessor(this));
	}
	start_my_vps();
}

void Daemon::broadcast_null() {

	list<VirtualProcessor*>::iterator it;

	for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
		vps_running.push_back(*it);
		(*it)->set_current_job(NULL);
		(*it)->resume();
		vps_waiting.pop_front();
	}
}

Daemon::~Daemon() {

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	vps_waiting.clear();
	vps_running.clear();
}

void Daemon::start_my_vps() {
	/* I still have the lock*/
	printf("DAEMON: Starting the vps\n");
	list<VirtualProcessor*>::iterator it;

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() == 0) {
			printf("DAEMON: Main_vp will be associated\n");
			AnahyVM::set_main_vp(*it);
			
		} else {
			(*it)->start(); // start vps
		}
	}

	printf("DAEMON: All vps has started\n");
}

void Daemon::stop_my_vps() {

	list<VirtualProcessor*>::iterator it;
	/* this allows the main VP to help the execution of
	 * remaining jobs and the Daemon to know that the
	 * main VP is also idle when there's no work
	 */

	printf("DAEMON: Stoping vps\n");
	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
	if(!vps_waiting.empty()) {
		for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
			if((*it)->get_id() > 0) {
				(*it)->stop();
			}
		}
	}
}

void Daemon::put_vp_on_waiting_list(VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	printf("Putting VP %d on waiting list\n", vp->get_id());
	
	vps_waiting.push_back(vp);
	vps_running.pop_front();

	pthread_mutex_unlock(&mutex);
}

void Daemon::answer_oldest_vp_waiting() {
	pthread_mutex_lock(&mutex);

	// job's state has already been set to running
	VirtualProcessor* vp;

	printf("Taking a vp from waiting list and searching a job for it\n");

	vp = vps_waiting.front();
	vps_waiting.pop_front();
	vps_running.push_back(vp);
	
	pthread_mutex_unlock(&mutex);

	request_job(NULL, vp);	// send a NULL job to
								// break VP loop
}

/**** PUBLIC METHODS ****/

void Daemon::post_job(Job* job) {
	pthread_mutex_lock(&mutex);
	
	graph->insert(job);

	printf("A job has been posted\n");

	VirtualProcessor* vp;
	if (!vps_waiting.empty()) {
		pthread_mutex_unlock(&mutex);
		printf("the waiting list is not empty:\n");
		answer_oldest_vp_waiting();
	}

	pthread_mutex_unlock(&mutex);
}

void Daemon::request_job(Job* _starting_job, VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	Job* job = NULL;

	job = graph->find_a_ready_job(_starting_job);
	
	if(job) {
		printf("Vp %d: I've found a job\n", vp->get_id());
		pthread_mutex_unlock(&mutex);
		
		vp->set_current_job(job);
		vp->resume();
	}
	else {

		if (vps_waiting.size() == num_vps-1) {
			printf("All vps are waiting. broadcasting null for all the vps\n");
			broadcast_null();
		}
		else {
			printf("Daemon didn't find a job for Vp %d, it will put the vp to waiting list\n", vp->get_id());
			pthread_mutex_unlock(&mutex);
			put_vp_on_waiting_list(vp);

		}
	}
	pthread_mutex_unlock(&mutex);
	return;
}

void Daemon::erase_job(Job* joined_job) {
	pthread_mutex_lock(&mutex);

	graph->erase(joined_job);

	pthread_mutex_unlock(&mutex);
}