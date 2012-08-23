#include "Agent.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

Agent::Agent(int _num_vps) : num_vps(_num_vps) {
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	
	//create my vps
	for (int i = 0; i < num_vps; i++) {
		vps_list.push_back(new VirtualProcessor(this));
	}
}

// this method won't use the mutex variable, because all vps are waiting 
// and will no longer need contentions
void Agent::broadcast_null() {
	
	list<VirtualProcessor*>::iterator it;
		
	for (it = request_list.begin(); it != request_list.end(); ++it) {
		vps_list.push_back(*it);
		it = request_list.erase(it);
	}

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		// this break the vp loop
		(*it)->set_current_job(NULL);
		(*it)->resume();
	}
}

Agent::~Agent() {

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	request_list.clear();
	vps_list.clear();
}

void Agent::start_my_vps() {

	list<VirtualProcessor*>::iterator it;

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		if((*it)->get_id() == 0) {
			//printf("Main_vp set.\n");
			AnahyVM::set_main_vp(*it);
			
		} else {
			(*it)->start(); // start vps
		}
	}
}

void Agent::stop_my_vps() {

	list<VirtualProcessor*>::iterator it;

	for (it = vps_list.begin(); it != vps_list.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
}

// this method answers the requests from vps from the request list
// make the stealing function
void Agent::attend_requests() {
	pthread_mutex_lock(&mutex);
	
	VirtualProcessor* vp = NULL;
	Job* job = NULL;
	list<VirtualProcessor*>::iterator it;

	while(!request_list.empty()) {

		if (request_list.size() == num_vps) {
			//all vps are waiting
			pthread_mutex_unlock(&mutex);
			broadcast_null();
			break;
		}
		else {
			vp = request_list.front();
			for (it = vps_list.begin(); it != vps_list.end(); ++it) {
				if(job = (*it)->request_job(NULL, true)) {
					break;
				}
			}
			if (job) {
				vp->set_current_job(job);
				vps_list.push_back(vp);
				request_list.pop_front();
				vp->resume();
			}
		}
		pthread_mutex_unlock(&mutex);
	}
}

void Agent::put_vp_on_request_list(VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	request_list.push_back(vp);
	vps_list.remove(vp);

	pthread_mutex_unlock(&mutex);
	//this modify the list of requests, then call the method that treat it
	attend_requests();
}