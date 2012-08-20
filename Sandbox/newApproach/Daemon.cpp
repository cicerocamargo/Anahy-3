#include "Daemon.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

/**** PRIVATE METHODS ****/

/* This method is too large and "ugly?"*/

Daemon::Daemon(int _num_vps) : num_vps(_num_vps) {
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	graph = new JobGraph();

	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	
	//printf("DAEMON: Creating vps\n");
	//create my vps
	for (int i = 0; i < num_vps; i++) {
		vps_running.push_back(new VirtualProcessor(this));
	}
}

void Daemon::broadcast_null() {

	//pthread_mutex_lock(&mutex);
	list<VirtualProcessor*>::iterator it;

	/* firstly, I've got to set all the current jobs of the vps on running 
	 * list as NULL, then I need to remove all the vps from waiting list
	 * and to set its current job as NULL, too.
	*/

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		(*it)->set_current_job(NULL);
	}

	for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
		
		(*it)->set_current_job(NULL);
		vps_running.push_back(*it);
		
		it = vps_waiting.erase(it);
	}
	pthread_cond_broadcast(&cond);
	//pthread_mutex_unlock(&mutex);
}

Daemon::~Daemon() {

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	vps_waiting.clear();
	vps_running.clear();

	delete graph;
}

void Daemon::start_my_vps() {
	/* I still have the lock*/
	//printf("DAEMON: Starting the vps\n");
	list<VirtualProcessor*>::iterator it;

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() == 0) {
			printf("DAEMON: Main_vp will be associated\n");
			AnahyVM::set_main_vp(*it);
			
		} else {
			(*it)->start(); // start vps
		}
	}

	//printf("DAEMON: All vps has started\n");
}

void Daemon::stop_my_vps() {

	list<VirtualProcessor*>::iterator it;
	/* this allows the main VP to help the execution of
	 * remaining jobs and the Daemon to know that the
	 * main VP is also idle when there's no work
	 */

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
	//printf("DAEMON: Vps stopped\n");
}

void Daemon::answer_oldest_vp_waiting() {

	// job's state has already been set to running
	VirtualProcessor* vp = NULL;
	Job* job = NULL;

	job = graph->find_a_ready_job(NULL);
	if (job) {
		vp = vps_waiting.front();
		vps_running.push_back(vp);
		vps_waiting.pop_front();
		
		//printf("DAEMON: Taking a VP %dfrom waiting list and sending a job to it run\n", vp->get_id());

		vp->set_current_job(job);
		/* The signal to cond variable needs to be 
		 * here in case of a job has been found. If not
		 * the vp waiting must to keep waiting.
		*/
		pthread_cond_signal(&cond);
	}
}

/**** PUBLIC METHODS ****/

// this method runs in a VP thread
void Daemon::post_job(Job* job) {
	pthread_mutex_lock(&mutex);
	
	graph->insert(job);

	//printf("DAEMON: A job has been posted\n");

	if (!vps_waiting.empty()) {
		
		//printf("DAEMON: the waiting list is not empty:\n");
		answer_oldest_vp_waiting();
	}

	pthread_mutex_unlock(&mutex);
}

void Daemon::request_job(Job* _starting_job, VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	Job* job = NULL;

	//printf("Daemon: I'll find a job to vp %d. w_list %lu and r_lis %lu\n", vp->get_id(), vps_waiting.size(), vps_running.size());
	job = graph->find_a_ready_job(_starting_job);
	
	if(job) {
		//printf("DAEMON: Vp %d: I've found a job\n", vp->get_id());
		
		vp->set_current_job(job);
	}
	else {

		if (vps_waiting.size() == num_vps-1) {
			//printf("DAEMON: All vps are waiting. Broadcasting null.\n");
			broadcast_null();
		}
		else {
			vps_waiting.push_back(vp);
			vps_running.remove(vp);
			pthread_cond_wait(&cond, &mutex);
		}
	}
	pthread_mutex_unlock(&mutex);
}

void Daemon::erase_job(Job* joined_job, VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	graph->erase(joined_job);
	vp->set_current_job(NULL);

	pthread_mutex_unlock(&mutex);
}