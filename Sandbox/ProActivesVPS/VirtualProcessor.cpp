#include "VirtualProcessor.h"
#include "Job.h"
#include "JobId.h"
#include "AnahyVM.h"
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>

int VirtualProcessor::tid_counter = 0;
int VirtualProcessor::instance_counter = 0;
int VirtualProcessor::idle_vps = 0;

pthread_key_t VirtualProcessor::key;
list<VirtualProcessor*> VirtualProcessor::vp_list;

void* VirtualProcessor::call_vp_run(void* arg) {
	associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;

	//here (later) we've got to set the affinity

	vp->run();
	return NULL;
}

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

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

void VirtualProcessor::call_vp_destructor(void* vp_obj) {
	//VirtualProcessor* vp = (VirtualProcessor*) vp_obj;
	//delete vp;
}

void VirtualProcessor::associate_vp_with_current_thread(void* vp_obj) {
	pthread_setspecific(key, vp_obj);
}

VirtualProcessor* VirtualProcessor::get_current_vp() {
	return (VirtualProcessor*) pthread_getspecific(key);
}

JobHandle VirtualProcessor::create_new_job(pfunc function, void* args) {
	
	JobId job_id(id, job_counter++);
	Job* job = new Job(job_id, current_job, this, function, args);

	JobHandle handle;
	handle.pointer = job;

	if (context_stack.size() > 5000) {
		job->run();
	} else {
		pthread_mutex_lock(&mutex);
			job_list.push_back(job);
		pthread_mutex_unlock(&mutex);
	}
	return handle;
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

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;

	while(!joined->compare_and_swap_state(finished, finished)) {
		suspend_current_job_and_run_another();
	}

	void* temp = joined->get_retval();

	delete joined;

	return temp;
}

Job* VirtualProcessor::get_job() {
	Job* job = NULL;
	pthread_mutex_lock(&mutex);
		if (!job_list.empty()) {

			job = job_list.back();
			job_list.pop_back();
		}
	pthread_mutex_unlock(&mutex);
	if (job) {
		job->compare_and_swap_state(ready, running);
	}
	return job;
}

/* interface with other VPs */
Job* VirtualProcessor::steal_job() {
	Job* job = NULL;
	pthread_mutex_lock(&mutex);
		if (!job_list.empty()) {

			job = job_list.front();
			job_list.pop_front();
		}
	pthread_mutex_unlock(&mutex);
	if (job) {
		job->compare_and_swap_state(ready, running);
	}
	return job;
}

void VirtualProcessor::start() {
	pthread_create(&thread, NULL, call_vp_run, this);
}

void VirtualProcessor::stop() {
	pthread_join(thread, NULL);
}