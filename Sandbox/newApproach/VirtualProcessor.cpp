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

/* PRIVATE METHODS */

void* VirtualProcessor::call_vp_run(void* arg) {
	VirtualProcessor::associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;
	vp->run();
	return NULL;
}

// 'job' was not Finished, so I'll ask for a ready child of the job (joined)
void VirtualProcessor::suspend_current_job_and_try_to_help(Job* joined) {
	context_stack.push(get_current_job()); // save context

	while(!joined->compare_and_swap_state(ready, ready)) {
		set_current_job(get_ready_job(joined, true));
	
		if (get_current_job()) {
			current_job->run();

			erase_job(get_current_job());	
		}
		else {
			/* there isn't a job to run */
			break;
		}
	}
	set_current_job(context_stack.top()); // restore stacked context
	
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
	
}

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
	printf("RUNNING_VP %d\n", id);
	while(true) {
		job = get_ready_job(NULL, true);

		if (!job) {
			printf("VP %d: Without job\n", id);
			/* the vp need ask a new job to Daemon */
			Daemon::waiting_for_a_job(this);
		}
		if (current_job) {
			current_job->compare_and_swap_state(ready, running);
			current_job->run();
		}
		else {
			break;
		}
	}
}

// dummy functon to fill pthread_key_create(...) requirements
void VirtualProcessor::call_vp_destructor(void *vp_obj) { }

void VirtualProcessor::associate_vp_with_current_thread(void* vp_obj) {
	printf("ASSOCIATING_VP\n");
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
	}
	else {
		attr = new JobAttributes();
	}

	Job* job = new Job(job_id, get_current_job(), this, attr, function, args);
	
	printf("VP %d: Creating job\n", id);
	insert_job(job);

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
		if (joined->compare_and_swap_state(ready, running)) {
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
		erase_job(joined);
	}
	return joined->get_retval();
}

// called from Daemon Thread
void VirtualProcessor::start() {
	// call this->run() in a new thread,
	// and put this in the thread specific memory
	pthread_mutex_unlock(&mutex); //unblock mutex that was block onto construtor
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

void VirtualProcessor::insert_job(Job* job) {
	pthread_mutex_lock(&mutex);
	
	graph.insert(job);

	pthread_mutex_unlock(&mutex);
}

Job* VirtualProcessor::get_ready_job(Job* _starting_job, bool normal_search) {
	printf("VP %d: Getting a ready job\n" ,id);
	Job* job = NULL;

	pthread_mutex_lock(&mutex);
	
	job = graph.find_a_ready_job(_starting_job, normal_search);
	
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