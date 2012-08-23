#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

Daemon* AnahyVM::daemon;
VirtualProcessor* AnahyVM::main_vp;

pthread_mutex_t AnahyVM::mutex;

void AnahyVM::start_vm() {
	daemon->start_my_vps(); // inside this method, the daemon sets the main VP

	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
}

void AnahyVM::stop_vm() {
	
	//printf("XXX Run main vp\n");
	//main_vp->run(); // this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work
	//printf("AnahyVM: The Daemon will stop all vps\n");
	daemon->stop_my_vps();
}

//here the interface begins to be described
void AnahyVM::init(int _num_vps) {
	
	daemon = new Daemon(_num_vps);

	VirtualProcessor::init_pthread_key();

	pthread_mutex_init(&mutex, NULL);

	start_vm();
	printf("**** Anahy3: Starting. Number of VPs equal %d...\n\n", _num_vps);
}

void AnahyVM::terminate() {

	stop_vm();

	VirtualProcessor::delete_pthread_key();

	delete daemon;
	printf("\n**** Anahy3: Done!\n");
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

void AnahyVM::set_main_vp(VirtualProcessor* vp) {
	main_vp = vp;
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