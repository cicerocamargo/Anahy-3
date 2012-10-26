#include "VirtualProcessor.h"
//#include "AnahyJob.h"
#include "AnahyVM.h"
#include <stdlib.h>

int VirtualProcessor::tid_counter = 0;
int VirtualProcessor::instance_counter = 0;
int VirtualProcessor::idle_vps = 0;

list<VirtualProcessor*> VirtualProcessor::vp_list;

void VirtualProcessor::run() {
	current_job = NULL;
	int num_vps = AnahyVM::get_num_vps();
	list<VirtualProcessor*>::iterator it;

	while(1) {

		current_job = this->get_job();

		if (!current_job) {  // no jobs found locally
	
			int __idle_vps = __sync_add_and_fetch(&idle_vps, 1);
			//printf("VP %d: thief_counter == %d\n", id, __idle_vps);
			if (__idle_vps == num_vps) {
				break;
			}

			for (it = vp_list.begin(); it != vp_list.end(); ++it) {
				if (*it != this) {
					current_job = (*it)->steal_job();
					if (current_job) {
						break;
					}
				}
			}
			__sync_sub_and_fetch(&idle_vps, 1);
		}
		
		if (current_job) {
			current_job->run();
		}
	}
}

VirtualProcessor::VirtualProcessor() {
	job_counter = 0;
	id = instance_counter++;

	if (tid_counter == AnahyVM::get_num_cpus()) {
		tid_counter = 0;
	}
	tid = tid_counter++;
	current_job = NULL;

	pthread_mutex_init(&mutex, NULL);
	vp_list.push_back(this);
}

VirtualProcessor::~VirtualProcessor() {
	pthread_mutex_destroy(&mutex);
	instance_counter--;
	vp_list.remove(this);
}



void VirtualProcessor::fork_job(AnahyJob* job) {

	job->set_parent(current_job);

	if (context_stack.size() > 10000) {
		job->run();
	} else {
		pthread_mutex_lock(&mutex);
		job_list.push_back(job);
		pthread_mutex_unlock(&mutex);
	}
}

void VirtualProcessor::suspend_current_job_and_run_another() {
	context_stack.push(current_job);
	//printf("VP # %d :: Context Stack Size # %d\n", this->get_id(), context_stack.size());

	list<VirtualProcessor*>::iterator it;

	// try to execute some other ready job
	current_job = this->get_job();

	if (!current_job) {  // no jobs found locally
		
		for (it = vp_list.begin(); it != vp_list.end(); ++it) {
		 	if (*it != this) {
				current_job = (*it)->steal_job();
				if (current_job) {
					break;
				}
			}
		}
	}

	if (current_job) {
		current_job->run();
	}
	
	current_job = context_stack.top();
	context_stack.pop();
}

void* VirtualProcessor::join_job(AnahyJob* job) {

	while(!job->compare_and_swap_state(AnahyJobStateFinished, AnahyJobStateFinished)) {
		suspend_current_job_and_run_another();
	}

	void* ret_val = job->result();	
	if(job->decrement_and_fetch_join_counter() == 0 && job->smart()) {
		delete job;
	}
	return ret_val;
}


AnahyJob* VirtualProcessor::get_job() {
	AnahyJob* job = NULL;

	pthread_mutex_lock(&mutex);

	if (!job_list.empty()) {
		job = job_list.back();
		job_list.pop_back();
	}

	pthread_mutex_unlock(&mutex);

	if (job) {
		job->compare_and_swap_state(AnahyJobStateReady, AnahyJobStateRunning);
	}
	return job;
}

/* interface with other VPs */
AnahyJob* VirtualProcessor::steal_job() {
	AnahyJob* job = NULL;
	
	pthread_mutex_lock(&mutex);
	
	if (!job_list.empty()) {
		job = job_list.front();
		job_list.pop_front();
	}
	
	pthread_mutex_unlock(&mutex);
	
	if (job) {
		job->compare_and_swap_state(AnahyJobStateReady, AnahyJobStateRunning);
	}
	return job;
}