#include "VirtualProcessor.h"
#include "Daemon.h"
//#include "AnahyVM.h" This no longer exist
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

// // 'job' was not Finished, so I'll ask daemon for a Ready job related to it
// void VirtualProcessor::suspend_current_job_and_try_to_help(Job* joined) {
// 	context_stack.push(current_job); // save context

// 	VPEvent event(GetJob, this, daemon, joined);
// 	daemon->push_event(event);
// 	block();

// 	if (current_job != context_stack.top()) { // daemon updated my current job
// 		current_job->run();

// 		VPEvent event(EndOfJob, this, daemon, current_job);
// 		daemon->push_event(event);
// 		current_job = context_stack.top(); // restore stacked context
// 	}
	
// 	// here, if daemon didn't change the current job and resumed me
// 	// (so I got here without executing the IF statement)
// 	// means that 'joined' got finished before a ready job was available
	
// 	context_stack.pop();
// }


// // run another job keeping track of the suspended job
// void VirtualProcessor::suspend_current_job_and_run_another(Job* another) {
// 	context_stack.push(current_job); // save context
// 	current_job = another; // update current_job

// 	current_job->run();

// 	VPEvent event(EndOfJob, this, daemon, current_job);
// 	daemon->push_event(event);
// 	current_job = context_stack.top(); // restore stacked context
// 	context_stack.pop();
	
// }

/* PUBLIC */

// called from Daemon Thread
VirtualProcessor::VirtualProcessor(Daemon* m) : daemon(m) {
	id = instance_counter++;
	current_job = NULL;
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
	
	Job* job;
	while (true) {
		if(!local_jobs.size()) {
			//IMPLEMENT WORK STEALING
		}
		current_job = local_jobs.pop_front();
		current_job->run();
		
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

	if (attr) {
		if (!attr->get_initialized()) {
			attr = new JobAttributes();
		}
	} else {
		attr = new JobAttributes();
	}

	Job* job = new Job(job_id, current_job, this, attr, function, args);
	
	local_jobs.push_back(job);

	JobHandle handle;
	handle.pointer = job;
	handle.id = job_id;
	return handle;
}

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;
	while (true) {
		if (joined->compare_and_swap_state(ready, running)) {
			// we are going to run the joined job... Hurray !!
			suspend_current_job_and_run_another(joined);
			break;
		}
		else {
			if (joined->compare_and_swap_state(finished, finished)) {
				break;
			}
			else {
				suspend_current_job_and_try_to_help(joined);
			}
		}
	}

	if (joined->dec_join_counter()) {
		VPEvent event(DestroyJob, this, daemon, handle.pointer);
		daemon->push_event(event);
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
	pthread_mutex_lock(&mutex);
}

// called from Daemon Thread
void VirtualProcessor::resume() {
	pthread_mutex_unlock(&mutex);	// unblock this vp's thread
}

/* getters and setters */

Job* VirtualProcessor::get_current_job() const {
	return current_job;
}

VirtualProcessor* VirtualProcessor::get_thief_vp() const {
	return thief_vp;
}

void VirtualProcessor::set_thief_vp(VirtualProcessor* vp) {
	thief_vp = vp;
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