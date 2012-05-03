#include "AnahyVM.h"
#include <cstdio>

/* STATIC MEMBERS' ITIALIZATIONS */
AnahyVM AnahyVM::unique_instance;
pthread_t AnahyVM::daemon_pthr;

/* PUBLIC METHODS' DEFINITIONS */

AnahyVM::AnahyVM() {
    
}

AnahyVM* AnahyVM::get_instance_handler() {
	return &unique_instance;
}

void* AnahyVM::run_vp(void* vp_obj) {

	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;

	//this set the key as a vp pointer
	pthread_setspecific(get_vp_key(), (void *) vp);
	
	vp->start_and_run();
	return NULL;
}

void* run_daemon(void* daemon_obj) {
	Daemon* daemon = (Daemon*) daemon_obj;
	daemon->start_and_run();

	return NULL;
}

void AnahyVM::boot(uint n_processors, sfunc scheduling_function) {
	puts("Booting AnahyVM...");
	num_processors = n_processors;
	vp_thread_array = (pthread_t*) malloc(n_processors*sizeof(pthread_t));

	VirtualProcessor::init_pthread_key();

	VirtualProcessor* vp;
	//list<VirtualProcessor*>::iterator it;
	
	daemon = new Daemon();
	processors.push_back(new VirtualProcessor(daemon, pthread_self()));

	//now we got to initializate the daemon thread
	pthread_create(&daemon_pthr, NULL, run_daemon, (void*)daemon);
	
	for (int i = 0; i < n_processors; i++) {
		vp = new VirtualProcessor(daemon, vp_thread_array[i]);
		processors.push_back(vp);
		pthread_create(&vp_thread_array[i], NULL, run_vp, (void*)vp);
	}
	
	puts("Done!");
}

void AnahyVM::shut_down() {
	puts("Shuting AnahyVM down...");
	
	for (int i = 0; i < num_processors; i++) {
		pthread_join(vp_thread_array[i], NULL);
	}
	pthread_join(daemon_pthr, NULL);
	
	free(vp_thread_array);
	delete(daemon);
}

void AnahyVM::insert_job(Job* job) {
	if (job->get_parent() == NULL) { // root job
		root_jobs.push_back(job);
	}
	job_map[job->get_id()] = job;
}

void AnahyVM::remove_job(Job* job) {
	
}

Job* AnahyVM::find_a_ready_job(Job* job) {
	
}

/* getters and setters */
list<Job*> AnahyVM::get_root_jobs() const {
	return root_jobs;
}

map<JobId,Job*> AnahyVM::get_job_map() const {
	return job_map;
}

int AnahyVM::get_num_processors() const {
	return num_processors;
}

vector<VirtualProcessor*> AnahyVM::get_processors() const {
	return processors;
}

Daemon* AnahyVM::get_daemon() const {
	return daemon;
}

Job* AnahyVM::get_job_by_id(JobId job_id) {
	return job_map[job_id];
}

void AnahyVM::set_scheduling_function(sfunc new_value) {
	scheduling_function = new_value;
}

pthread_key_t AnahyVM::get_vp_key() {
	return VirtualProcessor::get_pthread_key();
}