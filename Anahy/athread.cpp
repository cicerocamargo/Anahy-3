#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "definitions.h"
#include "athread.h"
#include "JobAttributes.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "AnahyVM.h"

AnahyVM* anahy = AnahyVM::get_instance_handler();

void help() {
	printf("\nOptions:\n");
	printf("	-h show this help message.\n");
	printf("	-d # set daemos' number to the environment;\n\t\t\t\t
		   Default is 1.\n");
	printf("	-v # set virtual processors' number per daemon to the environment;\n\t\t\t\t
		   Default is 1.\n");
	printf("	-s # set the scheduling function to execute the tasks\n\t\t\t\t
		   0-sched0; 1-sched1; 2-sched2; 3-sched3.\n");
	printf("	-m # specifies the operation mode.\n\t\t\t\t
		   1-PASSIVE; 2-NORMAL; 3-AGRESSIVE.\n");
	printf("\n");
}

void aInit(int argc, char** argv) {
	// parse command line
	// mode_operation: 1-PASSIVE, 2-SAME, 3-AGRESSIVE
	uint num_vps, num_daemons, scheduling_function, mode_operation;
	int c;
	while(1) {
		if((c = getopt(argc, argv, "hd:v:s:m:")) == -1)
			break;
		switch(c) {
			case 'd':
				num_daemons = strtol(optarg, NULL, 10);
				if(num_daemons < 1) {
					printf("Invalid number of daemons, assuming 1 daemon!\n");
					num_daemons = 1;
				}
			case 'v':
				num_vps = strtol(optarg, NULL, 10);
				if(num_vps < 1) {
					printf("Invalid number of vps, assuming 1 vp!\n");
					num_vps = 1;
				}
				break;
			case 's':
				scheduling_function = strtol(optarg, NULL, 10);
				if(scheduling_function < 0) {
					printf("Invalid scheduling function, assuming default function!\n");
					scheduling_function = 0;
				}
				break;
			case 'm':
				mode_operation = strtol(optarg, NULL, 10);
				if(mode_operation < 1 && mode_operation > 3) {
					printf("Invalid Operation mode, assuming mode as PASSIVE!\n");
					mode_operation = 1;
				}
				break;
			case 'h':
			default:
				help();
				break;
		}
	}

	anahy->boot(num_daemons, num_vps, scheduling_function, mode_operation);
}

void aTerminate() {
	anahy->shut_down();
}

void athread_exit(void* value_ptr) {
	pthread_key_t current_vp = VirtualProcessor::get_pthread_key();
	VirtualProcessor* vp = (VirtualProcessor*)pthread_getspecific(current_vp);
	Job* job = vp->get_current_job();
	job->set_retval(value_ptr);
}

int athread_create(athread_t* thid, athread_attr_t* attr, pfunc function, void* args) {

	pthread_key_t current_vp = VirtualProcessor::get_vp_key();
	VirtualProcessor* vp = (VirtualProcessor*)pthread_getspecific(current_vp);

	JobId job_id = vp->create_new_job(function, args, (JobAttributes) *attr);

	*thid = job_id;

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
	pthread_key_t current_vp = VirtualProcessor::get_vp_key();
	VirtualProcessor* vp = (VirtualProcessor*)pthread_getspecific(current_vp);
	// which job is this ?
	Job* job = anahy->get_job_by_id((JobId) thid);

	bool done = false;
	do {
		JobState state = compare_and_swap(job->get_state_var(), ready, running);

		if (state == running) {
			// if the joined job was already running ...
			// try to help, executing one of its descendants
			vp->suspend_current_job_and_try_to_help(job);
		}
		else {
			// if the job was ready for execution or already finished
			if (state == ready) {
				vp->suspend_current_job_and_run_another(job);
			}
			
			
			*result = job->get_retval();
			/*
			if (job->decrement_join_count() == 0) {
				delete job;
			}
			*/
			done = true;
		}
	} while (!done);
	return 0;
}

int athread_attr_init(athread_attr_t* attr) {
	//initialize all the job attributes as default
	JobAttributes* attr_t = new JobAttributes(1, true, ATHREAD_CREATE_JOINABLE, 0);
	*attr = attr_t;
	return 0;
}

int athread_attr_destroy(athread_attr_t* attr) {
	attr->initialized = false;
	return 0;
}

int athread_attr_setdetached(athread_attr_t* attr, int _detach_state) {
	if (!(detach_state & (ATHREAD_CREATE_JOINABLE | ATHREAD_CREATE_DETACHED))) {
		return -1;
	} else {
		attr->detach_state = _detach_state;
		return 0;
	}
}

int athread_attr_getdetached(athread_attr_t* attr, int* _detach_state) {
	*_detach_state = attr->get_JobAttributes_State();
	return 0;
}

int athread_attr_setjoinnumber(athread_attr_t* attr, int _max_joins) {
	attr->max_joins = _max_joins;
	return 0;
}

int athread_attr_getjoinnumber(athread_attr_t* attr, int* _max_joins) {
	*_max_joins = attr->get_max_joins();
	return 0;	
}

int athread_attr_setjobcost(athread_attr_t* attr, int _job_cost) {
	attr->job_cost = _job_cost;
	return 0;
}