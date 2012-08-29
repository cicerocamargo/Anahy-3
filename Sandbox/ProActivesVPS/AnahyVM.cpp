#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

VirtualProcessor* AnahyVM::main_vp;
list<VirtualProcessor*> AnahyVM::vps;
int AnahyVM::num_vps;
int AnahyVM::num_cpus;

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
		}
	}
}

//here the interface begins to be described
void AnahyVM::init(int _num_vps) {
	num_vps = _num_vps;
	VirtualProcessor::init_pthread_key();
	VirtualProcessor::init_vp_list(num_vps);

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

void AnahyVM::create(JobHandle* handle, JobAttributes* attr,
	pfunc function, void* args) {

	VirtualProcessor* vp = VirtualProcessor::get_current_vp();
	*handle = vp->create_new_job(function, args, attr);
}

void AnahyVM::join(JobHandle handle, void** result) {
	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	void* temp = vp->join_job(handle);

	if(result) {
		*result = temp;
	}
}

int attr_init(JobAttributes* attr) {
	//initialize all the job attributes as default
	JobAttributes* attr_t = new JobAttributes();
	attr = attr_t;
	return 0;
}

int attr_destroy(JobAttributes* attr) {
	attr->set_initialized(false);
	delete attr;
	return 0;
}

int attr_setdetached(JobAttributes* attr, JobAttributes_State _detach_state) {
	if (!(attr->get_JobAttributes_State() & (CREATE_JOINABLE | CREATE_DETACHED))) {
		return -1;
	} else {
		attr->set_detach_state(_detach_state);
		return 0;
	}
}

int attr_getdetached(JobAttributes* attr, JobAttributes_State* _detach_state) {
	*_detach_state = attr->get_JobAttributes_State();
	return 0;
}

int attr_setjoinnumber(JobAttributes* attr, int _num_joins) {
	if (_num_joins < 0) {
		return -1;
	}
	attr->set_num_joins(_num_joins);
	return 0;
}

int attr_getjoinnumber(JobAttributes* attr, int* _num_joins) {
	*_num_joins = attr->get_num_joins();
	return 0;	
}

int attr_setjobcost(JobAttributes* attr, int _job_cost) {
	if (_job_cost < 0) {
		return -1;
	}
	attr->set_job_cost(_job_cost);
	return 0;
}

int attr_setjobcost(JobAttributes* attr, int* _job_cost) {
	*_job_cost = attr->get_job_cost();
	return 0;
}