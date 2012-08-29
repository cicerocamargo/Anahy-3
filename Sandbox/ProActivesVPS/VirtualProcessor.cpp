#include "VirtualProcessor.h"
#include "Job.h"
#include "JobAttributes.h"
#include "JobId.h"
#include "AnahyVM.h"
#include <iostream>
#include <sched.h>


bool VirtualProcessor::end_of_program = false;
int VirtualProcessor::thief_counter = 0;
int VirtualProcessor::tid_counter = 0;
int VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;
VirtualProcessor** VirtualProcessor::vp_list;

void* VirtualProcessor::call_vp_run(void* arg) {
	associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;

	//here (later) we've got to set the affinity

	vp->run();
	return NULL;
}

void VirtualProcessor::run() {
	thief_mode = false;
	current_job = NULL;
	
	while(1) {
	
		if (end_of_program) {
			break;
		}

		current_job = this->get_job();

		if (!current_job) {  // no jobs found locally

			if (!thief_mode) {
				thief_mode = true;
				int __thief_counter = __sync_add_and_fetch(&thief_counter, 1);
				printf("VP %d: thief_counter == %d\n", id, __thief_counter);
				if (__thief_counter == AnahyVM::get_num_vps()) {
					printf("VP %d: FIM!\n", id);
					end_of_program = true;
					break;
				}
			}
				
			int num_vps = AnahyVM::get_num_vps();
			for (int i = 0; i < num_vps; ++i) {
				if (vp_list[i]!= this) {
					current_job = (vp_list[i])->get_job();
					if (current_job) {
						break;
					}
				}
			}
		}

		if (current_job) {
			if (thief_mode) {
				thief_mode = false;
				__sync_sub_and_fetch(&thief_counter, 1);
			}
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

	vp_list[id] = this;
}

VirtualProcessor::~VirtualProcessor() {
	pthread_mutex_destroy(&mutex);
}

void VirtualProcessor::init_vp_list(int num_vps) {
	if (vp_list) {
		free(vp_list);
	}
	vp_list = (VirtualProcessor**) malloc(num_vps*sizeof(VirtualProcessor*));
}

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

void VirtualProcessor::call_vp_destructor(void* vp_obj) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;
	delete vp;
}

void VirtualProcessor::associate_vp_with_current_thread(void* vp_obj) {
	pthread_setspecific(key, vp_obj);
}

VirtualProcessor* VirtualProcessor::get_current_vp() {
	return (VirtualProcessor*) pthread_getspecific(key);
}

JobHandle VirtualProcessor::create_new_job(pfunc function, void* args, JobAttributes* attr) {
	JobId job_id(id, job_counter++);
	// if (attr) {
	// 	if (!attr->get_initialized()) {
	// 		delete attr;
	// 		attr = new JobAttributes();
	// 	}
	// } else {
	// 	attr = new JobAttributes();
	// }
	Job* job = new Job(job_id, current_job, this, attr, function, args);

	JobHandle handle;
	handle.pointer = job;

	pthread_mutex_lock(&mutex);
		job_list.push_back(job);
	pthread_mutex_unlock(&mutex);

	return handle;
}

void VirtualProcessor::suspend_current_job_and_run_another() {
	context_stack.push(current_job);

	// try to execute some other ready job

	current_job = this->get_job();

	if (!current_job) {  // no jobs found locally
		if (!thief_mode) {
			thief_mode = true;
			int __thief_counter = __sync_add_and_fetch(&thief_counter, 1);
			if (__thief_counter == AnahyVM::get_num_vps()) {
				printf("ooops...\n");
				abort();
			}
		}
		
		int num_vps = AnahyVM::get_num_vps();
		for (int i = 0; i < num_vps; ++i) {
		 	if (vp_list[i]!= this) {
					current_job = (vp_list[i])->get_job();
					if (current_job) {
						break;
					}
				}
		}
	}
	if (current_job) {
		if (thief_mode) {
			thief_mode = false;
			__sync_sub_and_fetch(&thief_counter, 1);
		}	
		current_job->run();
	}

	current_job = context_stack.top();
	context_stack.pop();
}

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;

	while(joined->get_state() != finished) {
		suspend_current_job_and_run_another();
	}

	return joined->get_retval();
}

/* interface with other VPs */
Job* VirtualProcessor::get_job() {
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