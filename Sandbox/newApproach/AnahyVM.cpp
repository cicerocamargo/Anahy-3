#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

Daemon* AnahyVM::daemon;
VirtualProcessor* AnahyVM::main_vp;

pthread_mutex_t AnahyVM::mutex;

void AnahyVM::start_vm() {
	daemon->start(); // start the daemon
		
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set
								// by daemon 0
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	printf("Main_vp associated\n");
	
	pthread_mutex_unlock(&mutex);
}

void AnahyVM::stop_vm() {
	
	main_vp->run(); // this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work

	daemon->stop();
}

//here the interface begins to be described
void AnahyVM::init(int _num_vps) {
	
	daemon = new Daemon(_num_vps);

	VirtualProcessor::init_pthread_key();

	pthread_mutex_init(&mutex, NULL);

	/* since the main thread has the VM's lock,
	* it can block itself in the next call
	* to wait for the VP 0 to be associated
	* with the main thread*/
	pthread_mutex_lock(&mutex);	// since the main thread has the VM's lock,
								// it can block itself in the next call
								// to wait for the VP 0 to be associated
								// with the main thread

	start_vm();
	printf("AnahyVM: Init done\n");
}

void AnahyVM::terminate() {

	stop_vm();

	VirtualProcessor::delete_pthread_key();

	delete daemon;
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

	printf("AnahyVM: A new job has been created\n");
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
	pthread_mutex_unlock(&mutex);
}