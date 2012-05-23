#include "VirtualProcessor.h"
#include "Daemon.h"
#include "AnahyVM.h"
#include "Job.h"
#include "JobAttributes.h"
#include <cstdio>
#include <cstdlib>

/**** STATIC MEMBERS' ITIALIZATIONS ****/

uint VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;


/* PRIVATE METHODS */

void* VirtualProcessor::call_vp_run(void* arg) {
	VirtualProcessor::associate_vp_with_current_thread(arg);
	VirtualProcessor* w = (VirtualProcessor*) arg;
	w->run();
	return NULL;
}

/*
// 'job' was not Finished, so I'll ask daemon for a Ready job related to it
void VirtualProcessor::suspend_current_job_and_try_to_help(Job* joined) {
	context_stack.push(current_job); // save context
	daemon->get_job(this, joined); // ask daemon and wait

	if (current_job != context_stack.top()) { // daemon updated my current job
		current_job->run();
		daemon->end_of_job(this, current_job); // notify daemon
		current_job = context_stack.top(); // restore stacked context
	}
	
	// when daemon don't change the current job and resume me
	// (so I got here without executing the IF statement)
	// means that 'joined' got finished before a ready job was available
	
	context_stack.pop();
}


// run another job keeping track of the suspended job
void VirtualProcessor::suspend_current_job_and_run_another(Job* another) {
	context_stack.push(current_job); // save context
	current_job = another; // update current_job
	
	current_job->run();
	daemon->end_of_job(this, current_job); // notify daemon
	
	current_job = context_stack.top(); // restore stacked context
	context_stack.pop();
	
}

*/

/* PUBLIC */

// called from Daemon Thread
VirtualProcessor::VirtualProcessor(Daemon* m) : daemon(m) {
	id = instance_counter++;
	current_job = NULL;
	job_counter = 0;

	// initializing my tabs string
	int i, num_tabs = id*2;
	tabs = (char*) malloc((num_tabs+1)*sizeof(char));
	for (i = 0; i < num_tabs; ++i) {
		tabs[i] = '\t';
	}
	tabs[i] = '\0';

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
}

// called from Daemon Thread
VirtualProcessor::~VirtualProcessor()  {
	puts("VirtualProcessor object deleted!");
	free(tabs);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void VirtualProcessor::run() {
	bool should_create_more_work = false;
	while (true) {
		daemon->get_job(this, NULL);
		
		if (!current_job) {
			say("going home.");
			break;
		}

		say("running");
		current_job->run();
		// current_job->compare_and_swap_state(running, finished);
		daemon->end_of_job(this, current_job);
		current_job = NULL;
	}
}

/**** STATIC METHODS ****/

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

// dummy functon to fill pthread_key_create(...) requirements
void VirtualProcessor::call_vp_destructor(void *vp_obj) { }

void VirtualProcessor::associate_vp_with_current_thread(void* vp_obj) {
	pthread_setspecific(key, vp_obj);
}

VirtualProcessor* VirtualProcessor::get_current_vp() { // class method!
	return (VirtualProcessor*) pthread_getspecific(key);
}

JobHandle VirtualProcessor::create_new_job(pfunc function, void* args,
		JobAttributes* attr) {
	JobId job_id(id, job_counter++);
	Job* job = new Job(job_id, current_job, this, attr, function, args);
	daemon->new_job(this, job);
	JobHandle handle;
	handle.pointer = job;
	handle.id = job_id;
	return handle;
}

// called from Daemon Thread
void VirtualProcessor::start() {
	// call this->run() in a new thread,
	// and put this in the thread specific memory
	pthread_create(&thread, NULL, call_vp_run, this);
	printf("Starting a VP %d on thread (%lu).\n", id, (long) thread);
}

// called from Daemon Thread
void VirtualProcessor::stop() {
	pthread_join(thread, NULL);
}

// called from a daemon Object
void VirtualProcessor::block() {
	pthread_mutex_lock(&mutex);
}

// called from Daemon Thread
void VirtualProcessor::resume() {
	pthread_mutex_unlock(&mutex);	// unblock this vp's thread
}

void VirtualProcessor::say(const char* str) {
	printf("%sVP %d is %s ", tabs, id, str);
	if (current_job) {
		current_job->get_id().display();
	}
	printf("\n");
}

// this is called from a Daemon thread

/* getters and setters */

Job* VirtualProcessor::get_current_job() const {
	return current_job;
}

void VirtualProcessor::set_current_job(Job* new_value) {
	current_job = new_value;
}

uint VirtualProcessor::get_id() const {
	return id;
}

ulong VirtualProcessor::get_job_counter() const {
	return job_counter;
}