#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "definitions.h"
#include "athread.h"
#include "JobAttributes.h"
#include "Agent.h"
#include "VirtualProcessor.h"
#include "AnahyVM.h"

void help() {
	printf("\nOptions:\n");
	printf("	-h show this help message.\n");
	printf("	-d # set daemos' number to the environment;\n\t\t\t\tDefault is 1.\n");
	printf("	-v # set virtual processors' number per daemon to the environment;\n\t\t\t\tDefault is 1.\n");
	printf("	-s # set the scheduling function to execute the tasks\n\t\t\t\t0-sched0; 1-sched1; 2-sched2; 3-sched3.\n");
	printf("	-m # specifies the operation mode.\n\t\t\t\t1-PASSIVE; 2-NORMAL; 3-AGRESSIVE.\n");
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

	AnahyVM::init(num_daemons, num_vps, scheduling_function, mode_operation);
}

void aTerminate() {
	AnahyVM::terminate();
}

void athread_exit(void* value_ptr) {
	VirtualProcessor* current_vp = VirtualProcessor::get_current_vp();
	Job* job = current_vp->get_current_job();
	job->set_retval(value_ptr);
}

int athread_create(athread_t* thid, athread_attr_t* attr, pfunc function, void* args) {
	VirtualProcessor* current_vp = VirtualProcessor::get_current_vp();
	if (attr) {
		if (!attr->get_initialized()) {
			//sprintf(stderr, "job attributes must be initialized\n");
			athread_attr_init(attr);
		}
	} else {
		athread_attr_init(attr);
	}
	JobHandle handle = current_vp->create_new_job(function, args, attr);
	*thid = handle;
	return 0;
}

int athread_join(athread_t thid, void** result) {
	VirtualProcessor* current_vp = VirtualProcessor::get_current_vp();
	*result = current_vp->join_job((JobHandle) thid);
	return 0;
}

int athread_attr_init(athread_attr_t* attr) {
	//initialize all the job attributes as default
	JobAttributes* attr_t = new JobAttributes();
	attr = attr_t;
	return 0;
}

int athread_attr_destroy(athread_attr_t* attr) {
	attr->set_initialized(false);
	delete attr;
	return 0;
}

int athread_attr_setdetached(athread_attr_t* attr, JobAttributes_State _detach_state) {
	if (!(attr->get_JobAttributes_State() & (ATHREAD_CREATE_JOINABLE | ATHREAD_CREATE_DETACHED))) {
		return -1;
	} else {
		attr->set_detach_state(_detach_state);
		return 0;
	}
}

int athread_attr_getdetached(athread_attr_t* attr, JobAttributes_State* _detach_state) {
	*_detach_state = attr->get_JobAttributes_State();
	return 0;
}

int athread_attr_setjoinnumber(athread_attr_t* attr, int _num_joins) {
	if (_num_joins < 0) {
		return -1;
	}
	attr->set_num_joins(_num_joins);
	return 0;
}

int athread_attr_getjoinnumber(athread_attr_t* attr, int* _num_joins) {
	*_num_joins = attr->get_num_joins();
	return 0;	
}

int athread_attr_setjobcost(athread_attr_t* attr, int _job_cost) {
	if (_job_cost < 0) {
		return -1;
	}
	attr->set_job_cost(_job_cost);
	return 0;
}

int athread_attr_setjobcost(athread_attr_t* attr, int* _job_cost) {
	*_job_cost = attr->get_job_cost();
	return 0;
}