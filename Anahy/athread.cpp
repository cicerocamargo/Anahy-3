#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "definitions.h"
#include "athread.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "AnahyVM.h"



uint num_vps;
AnahyVM* anahy = AnahyVM::get_instance_handler();

Job* work_stealing(list<Job*> list, Job* job) {
	return NULL;
}

void* vp_run(void* vp_args) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_args;
	vp->start();
	return NULL;
}

void aInit(int argc, char** argv) {
/*
	int c;
	while(1) {
		if((c = getopt(argc, argv, "v:s:m:")) == -1)
			break;
		switch(c) {
			case 'v':
				num_vps = strtol(optarg, NULL, 10);
				if(num_vps < 1) {
					printf("Invalid number of vps, assuming 1 vp!\n");
					num_vps = 1;
				}
				break;
			case 's':
				//set scheduler (our problem)
				break;
			case 'm':
				// set mode to executing and the 
				// frequency of the cores (Alan says my problem)
				break;
			default:
				//set default state and frequency of the cores
				break;
		}
	}
*/
	
	anahy->boot(1, work_stealing);
	//printf("Anahy started: %d VPs running.\n", num_vps);
	// do something with AnahyVM object
	
}

void aTerminate() {
	anahy->shut_down();	
}

void athread_exit(void* value_ptr) {
	VirtualProcessor* vp = VirtualProcessor::get_vp_from_pthread(pthread_self());
	Job* job = vp->get_current_job();
	job->set_retval(value_ptr);
}

int athread_create(athread_t* thid, athread_attr_t* attr, pfunc function, void* args) {
	
	VirtualProcessor* vp = VirtualProcessor::get_vp_from_pthread(pthread_self());
	JobId job_id = vp->get_new_JobId();
	Job* parent = vp->get_current_job();
	JobAttributes job_attr = (long int) attr;
	Job* job = new Job(job_id, parent, vp, job_attr, function, args);

	vp->notify_new_job(job);

	*thid = job_id;
	
	printf("New Job!\n");
	job->display();
	anahy->insert_job(job);

	return 0;
}

// to be changed!
int compare_and_swap(JobState* state, JobState target_val, JobState new_val) {
	if (*state == target_val) {
		*state = new_val;
		return target_val;
	}
	else {
		return *state;
	}
}

int athread_join(athread_t thid, void** result) {
	// which vp is this ?
	VirtualProcessor* vp = VirtualProcessor::get_vp_from_pthread(pthread_self());
	// which job is this ?
	Job* job = anahy->get_job_by_id((JobId) thid);

	bool done = false;
	do {
		JobState state = compare_and_swap(job->get_state_var(), ready, running);

		if (state == running) {
			// if the joined job was already running ...
			// try to help, executing one of its descendants
			vp->try_to_help_job(job);
		}
		else {
			// if the job was ready for execution or already finished
			if (state == ready) {
				vp->run_temp_job(job);
			}
			
			*result = job->get_retval();
			done = true;
		}
	} while (!done);
	return 0;
}

