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

//here the interface begins to be described	
void AnahyVM::init(int argc, char **argv) {
	int c;

	num_vps = 1;
	if (argc == 2) { // if we have only two arguments and the second one asks for help or the version...
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			help();
			exit(0);
		} else if (strcmp(argv[1], "--version") == 0) {
			printf("\nAnahy %.1f (Beta) runtime.\n\n", VERSION);
			exit(0);
		}
	} 
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-v") == 0 && i+1 < argc) {
			// we have v and no more arguments
			int _num_vps = atoi(argv[++i]);
			num_vps = _num_vps > 1 ? _num_vps : 1;
			break;
		}
	}

	VirtualProcessor::init_pthread_key();

	for(int i = 0; i < num_vps; i++) {
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

void AnahyVM::_exit(void* value_ptr) {
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