#include "include/AnahyVM.h"
#include "include/VirtualProcessor.h"
#include "include/JobAttributes.h"
#include <cstdio>
#include <cstdlib>

VirtualProcessor* AnahyVM::main_vp;
list<VirtualProcessor*> AnahyVM::vps;
int AnahyVM::num_vps;
int AnahyVM::num_cpus;

// SYSTEM MANAGEMENT INTERFACE

void AnahyVM::start_vps() {
	list<VirtualProcessor*>::iterator it;

	for (it = vps.begin(); it != vps.end(); ++it) {
		if ((*it)->get_id() == 0) {
			main_vp = *it;
		} else {
			(*it)->start();
		}
	}
}

void AnahyVM::stop_vps() {
	list<VirtualProcessor*>::iterator it;

	for (it = vps.begin(); it != vps.end(); ++it) {
		if ((*it)->get_id() != 0) {
			(*it)->stop();
			num_vps--;
		}
	}
}

void AnahyVM::help() {
	printf("\nOptions:\n");
	printf("	-h show this help message.\n");
	printf("	-v # set virtual processors to the environment;\n\t\t\t\tDefault is 1.\n");
	printf("\n");
}

// USER THREADS INTERFACE

//here the interface begins to be described	
void AnahyVM::init(int argc, char **argv) {
	
	int _num_vps;

	int c;
	while(1) {
		if ((c = getopt(argc, argv, "hv:")) == -1)
				break;
		switch(c) {
			case 'v': 
				_num_vps = strtol(optarg, NULL, 10);
				if (_num_vps < 1) {
					printf("\tNumber of vps cannot be negavite, assuming 1\n");
					_num_vps = 1;
				}
				printf("\tInitializing Anahy3 with num. of vps = %d\n\n", _num_vps);
			break;
			default:
				help();
				break;
		}
	}

	num_vps = _num_vps;
	VirtualProcessor::init_pthread_key();

	for(int i = 0; i < _num_vps; i++) {
		vps.push_back(new VirtualProcessor());
	}

	start_vps();

	VirtualProcessor::associate_vp_with_current_thread(main_vp);
}

void AnahyVM::terminate() {

	main_vp->run();// this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work

	stop_vps();

	VirtualProcessor::delete_pthread_key();
	vps.clear();
}

void AnahyVM::exit(void* value_ptr) {
	VirtualProcessor* current_vp = VirtualProcessor::get_current_vp();
	Job* job = current_vp->get_current_job();
	job->set_retval(value_ptr);
}

void AnahyVM::create(athread_t* handle, athread_attr_t* attr, pfunc function, void* args) {

	VirtualProcessor* vp = VirtualProcessor::get_current_vp();
	JobAttributes* _attr;
	//we need that job's attributes has been initialized, because
	//we will need it to decrement the number of job's joins
	if (!attr) {
		_attr = new JobAttributes();
	} else {
		_attr = attr;
	}
	*handle = vp->create_new_job(function, _attr, args);
}

void AnahyVM::join(athread_t handle, void** result) {
	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	void* temp = vp->join_job(handle);

	if(result) {
		*result = temp;
	}
}

// USER ATTRIBUTES INTERFACE

void AnahyVM::attr_init(JobAttributes* attr) {
	JobAttributes* _attr = new JobAttributes();
	attr = _attr;
}

void AnahyVM::attr_setjobcost(JobAttributes* attr, JobCost cost) {
	attr->set_job_cost(cost);
}

void AnahyVM::attr_setjobjoins(JobAttributes* attr, int joins) {
	attr->set_num_joins(joins);
}

int AnahyVM::attr_getjobcost(JobAttributes* attr) {
	return attr->get_job_cost();
}