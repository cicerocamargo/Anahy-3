#include "Agent.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

/**** PRIVATE METHODS ****/

Agent::Agent(int _num_vps) : num_vps(_num_vps) {
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	
	//create my vps
	for (int i = 0; i < num_vps; i++) {
		vps_list.push_back(new VirtualProcessor(this));
	}
}

void Agent::broadcast_null() {

	pthread_mutex_lock(&mutex);
	list<VirtualProcessor*>::iterator it;

	/* firstly, I've got to set all the current jobs of the vps on list 
	 * list as NULL, then I need to remove all the vps from waiting list
	 * and to set its current job as NULL, too.
	*/

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		(*it)->set_current_job(NULL);
	}

	for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
		
		(*it)->set_current_job(NULL);
		vps_list.push_back(*it);
		
		it = vps_waiting.erase(it);
	}
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
}

Agent::~Agent() {

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	vps_waiting.clear();
	vps_list.clear();
}

void Agent::start_my_vps() {
	//printf("DAEMON: Starting the vps\n");
	list<VirtualProcessor*>::iterator it;

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		if((*it)->get_id() == 0) {
			printf("DAEMON: Main_vp set.\n");
			AnahyVM::set_main_vp(*it);
			
		} else {
			(*it)->start(); // start vps
		}
	}

	//printf("DAEMON: All vps has started\n");
}

void Agent::stop_my_vps() {

	list<VirtualProcessor*>::iterator it;
	/* this allows the main VP to help the execution of
	 * remaining jobs and the Agent to know that the
	 * main VP is also idle when there's no work
	 */

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
	//printf("DAEMON: Vps stopped\n");
}

void Agent::put_vp_on_waiting_list(VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	//printf("DAEMON: Putting VP %d on waiting list\n", vp->get_id());
	//printf("%lu R_list and %lu to W_list\n", vps_list.size(), vps_waiting.size());

	list<VirtualProcessor*>::iterator it;

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		if ((*it)->get_id() == vp->get_id()) {
			break;
		}
	}

	vps_waiting.push_back(*it);
	it = vps_list.erase(it);

	pthread_mutex_unlock(&mutex);
}

void Agent::take_vp_from_waiting_list(Job* vp) {

	VirtualProcessor* vp = NULL;

	vp = vps_waiting.front();
	vps_list.push_back(vp);
	vps_waiting.pop_front();

	vp->set_current_job(job);

}

//temp variable can be to use to order some control
bool Agent::answer_oldest_vp_waiting() {
	// job's state has already been set to list

	VirtualProcessor* vp = NULL;
	Job* job = NULL;

	vp = vps_waiting.front()

	job = vp->find_a_ready_job(NULL, NULL);
	if (job) {
		take_vp_from_waiting_list(job);
		/* The signal to cond variable needs to be 
		 * here in case of a job has been found. If not
		 * the vp waiting must to keep waiting.
		*/
		pthread_cond_signal(&cond);
		return true;
	}
	return false;
}

/**** PUBLIC METHODS ****/

bool Agent::wake_up_some_waiting_vp() {
	pthread_mutex_lock(&mutex);
	bool temp = false;
	
	if (!vps_waiting.empty()) {
		
		temp = answer_oldest_vp_waiting();
	}
	pthread_mutex_unlock(&mutex);
	return temp;
}

Job* Agent::work_stealing_function(VirtualProcessor* vp) {

	Job* job = NULL;

	//printf("Agent: I'll find a job to vp %d. w_list %lu and r_lis %lu\n", vp->get_id(), vps_waiting.size(), vps_list.size());
	list<VirtualProcessor*>::interator it;
	for (it = vps_list.begin(); i != vps_list.end(); ++it) {
		if(job = (*it)->find_a_ready_job(_starting_job, true)) {
			break;
		}
	}	
	
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
			//printf("DAEMON: I didn't find a job for Vp %d, it will put the vp to waiting list\n", vp->get_id());
			/* One vp can put itself on waiting lista after 
			 * the last comparison, then I need to compare again to I don't 
			 * put one vp on waiting list and no longer take it from there*/

			if (vps_waiting.size() != num_vps-1) {
				put_vp_on_waiting_list(vp);
				pthread_cond_wait(&cond, &mutex);
			}
			else {
				broadcast_null();
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}