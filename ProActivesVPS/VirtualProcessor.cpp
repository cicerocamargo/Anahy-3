#include "include/VirtualProcessor.h"
#include "include/Job.h"
#include "include/JobId.h"
#include "include/AnahyVM.h"

//#include <sched.h>
#include <stdlib.h>

int VirtualProcessor::tid_counter = 0;
int VirtualProcessor::instance_counter = 0;
int VirtualProcessor::idle_vps = 0;

pthread_key_t VirtualProcessor::key;
list<VirtualProcessor*> VirtualProcessor::vp_list;

void* VirtualProcessor::call_vp_run(void* arg) {
	associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;

	// Here is where the system sets the processor affinity,
	// this is done from a circular core's list. Example: 4 vps to 8 cores
	// 0_VP -> 0_CORE ... 3_VP -> 3_CORE -> 4_VP -> 0_CORE ... n-1_VP -> m-1_CORE

/*
	cpu_set_t cpuset;
	int thr = pthread_self();
	int this_tid = vp->get_tid();
	int cpuset_len = sizeof(cpu_set_t);
	
	CPU_ZERO(&cpuset);
	//printf("%u -- %d\n", vp->get_id(), this_tid);

	CPU_SET(this_tid, &cpuset);

	if (pthread_setaffinity_np(thr, cpuset_len, &cpuset) != 0) {
		//printf("Error in pthread_setaffinity_np!\n");
	}
*/

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

JobHandle VirtualProcessor::create_new_job(pfunc function, JobAttributes* attr, void* args) {
	
	JobId job_id(id, job_counter++);
	Job* job = new Job(job_id, current_job, this, attr, function, args);

	JobHandle handle;
	handle.pointer = job;

	if (context_stack.size() > 10000) {
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
	
	//we got to verify if the joined job has joins, yet
	JobAttributes* job_attr = joined->get_attributes();
	
	if(job_attr->dec_join_counter()) {
		delete joined;
	}
	return temp;
}


//let's divide the search function in two methods, because
//is not good to pass the job cost as an argument to return a
//job, the method should indicates its action and not its arguments

Job* VirtualProcessor::searchMinJobCost() {
	list<Job*>::reverse_iterator rit;
	Job* job = NULL;
	JobAttributes* attr;
	pthread_mutex_lock(&mutex);

	for(rit = job_list.rbegin(); rit != job_list.rend(); ++rit) {
		attr = (*rit)->get_attributes();
		if (attr->get_job_cost() == MINIMUM_COST) {
			job = *rit;
			break;
		}
	}
	if (job) {
		job_list.remove(*rit);
	}
	pthread_mutex_unlock(&mutex);
	return job;
}

Job* VirtualProcessor::searchMaxJobCost() {
	list<Job*>::reverse_iterator rit;
	Job* job = NULL;
	JobAttributes* attr;
	pthread_mutex_lock(&mutex);

	for(rit = job_list.rbegin(); rit != job_list.rend(); ++rit) {
		attr = (*rit)->get_attributes();
		if (attr->get_job_cost() == MAXIMUM_COST) {
			job = *rit;
			break;
		}
	}
	if (job) {
		job_list.remove(*rit);
	}
	pthread_mutex_unlock(&mutex);
	return job;
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