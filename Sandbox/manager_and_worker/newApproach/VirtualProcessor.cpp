#include "VirtualProcessor.h"
#include "Daemon.h"
#include "Job.h"
#include "JobAttributes.h"
#include "JobId.h"
#include <cstdio>
#include <cstdlib>

/**** STATIC MEMBERS' ITIALIZATIONS ****/

pthread_mutex_t VirtualProcessor::mutex;
uint VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;
bool VirtualProcessor::blocked = false;
/* PRIVATE METHODS */

void* VirtualProcessor::call_vp_run(void* arg) {
	VirtualProcessor::associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;
	vp->run();
	return NULL;
}

// 'job' was not Finished, so I'll ask daemon for a Ready job related to it
/*void VirtualProcessor::suspend_current_job_and_try_to_help(Job* joined) {
	context_stack.push(get_current_job()); // save context

	set_current_job(get_job());
	block();

	if (get_current_job() != context_stack.top()) { // daemon updated my current job
		current_job->run();

		erase_job(get_current_job());
		set_current_job(context_stack.top()); // restore stacked context
	}
	
	// here, if daemon didn't change the current job and resumed me
	// (so I got here without executing the IF statement)
	// means that 'joined' got finished before a ready job was available
	
	context_stack.pop();
}


// run another job keeping track of the suspended job
void VirtualProcessor::suspend_current_job_and_run_another(Job* another) {
	context_stack.push(get_current_job()); // save context
	set_current_job(another); // update current_job

	current_job->run();

	erase_job(get_current_job());
	set_current_job(context_stack.top()); // restore stacked context
	context_stack.pop();
	
}*/

/**** STATIC METHODS ****/

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

/**** PUBLIC METHODS ****/

// called from Daemon Thread
VirtualProcessor::VirtualProcessor() {
	id = instance_counter++;
	set_current_job(NULL);
	job_counter = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
}

// called from Daemon Thread
VirtualProcessor::~VirtualProcessor()  {
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

void VirtualProcessor::run() {
	Job* job = NULL;
	while(true) {
		job = get_ready_job(NULL);
		if (!job) {
			/* the vp need ask a new job to Daemon */
			Daemon::waiting_for_a_job(this);
		}
		if (get_current_job()) {
			current_job->compare_and_swap_state(ready, running);
			current_job->run();
			set_current_job(NULL);
		}
	}
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

	if (attr) {
		if (!attr->get_initialized()) {
			attr = new JobAttributes();
		}
	} else {
		attr = new JobAttributes();
	}

	Job* job = new Job(job_id, get_current_job(), this, attr, function, args);
	
	insert_job(job);
	/* if I'm on waiting list, I've got to ask to Daemon
	to remove me from there, because a new job has posted
	on my local list
	*/
	if(get_status()) {
		Daemon::remove_vp_from_waiting_list(this);
	}

	JobHandle handle;
	handle.pointer = job;
	return handle;
}

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;
	VirtualProcessor* vp_thief;
	/*(job->get_vp_thief() == NULL) means this job hasn't stolen */
	vp_thief = joined->get_vp_thief();
	while (true) {

	}

	if (joined->dec_join_counter()) {
		if(vp_thief) {
			vp_thief->erase_job(joined);
		}
		else {
			erase_job(joined);
		}
	}
	return joined->get_retval();
}

// called from Daemon Thread
void VirtualProcessor::start() {
	// call this->run() in a new thread,
	// and put this in the thread specific memory
	pthread_create(&thread, NULL, call_vp_run, this);
}

// called from Daemon Thread
void VirtualProcessor::stop() {
	pthread_join(thread, NULL);
}

// called from a daemon Object
void VirtualProcessor::block() {
	set_status(true);
	pthread_mutex_lock(&mutex);
}

// called from Daemon Thread
void VirtualProcessor::resume() {
	set_status(false);
	pthread_mutex_unlock(&mutex);	// unblock this vp's thread
}

void VirtualProcessor::insert_job(Job* job) {
	pthread_mutex_lock(&mutex);
	
	graph.insert(job);

	pthread_mutex_unlock(&mutex);
}

Job* VirtualProcessor::get_ready_job(Job* _starting_job) {
	pthread_mutex_lock(&mutex);

	Job* job = NULL;
	job = graph.find_a_ready_job(_starting_job);

	pthread_mutex_unlock(&mutex);
	return job;
}

void VirtualProcessor::erase_job(Job* joined_job) {
	pthread_mutex_lock(&mutex);

	graph.erase(joined_job);

	pthread_mutex_unlock(&mutex);
}

uint VirtualProcessor::get_id() const {
	return id;
}

ulong VirtualProcessor::get_job_counter() const {
	return job_counter;
}