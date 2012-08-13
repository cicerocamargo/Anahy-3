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

/**** STATIC METHODS ****/

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

/**** PUBLIC METHODS ****/

// called from Daemon Thread
VirtualProcessor::VirtualProcessor(Daemon* _daemon) : daemon(_daemon) {
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

// this is the main loop of vp
void VirtualProcessor::run() {

	printf("VP %d: Running...\n", id);
	while(true) {
		daemon->request_job(NULL, this);
		block();
		
		if (!get_current_job()) {
			printf("VP %d: I have no job, I'll stop\n", id);
			break;
		}
		else {
			current_job->run();
			set_current_job(NULL);
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
	
	daemon->post_job(job);
	printf("VP %d sended a job to daemon graph\n", id);
	
	JobHandle handle;
	handle.pointer = job;
	return handle;
}

/*the job was not Finished, maybe it's waiting its children to finish,
 * so, I'll help that job asking for a ready child while it doesn't 
 be Finished.
 */
void VirtualProcessor::suspend_current_job_and_try_to_help(Job* joined) {
	context_stack.push(current_job); // save context

	while(joined->compare_and_swap_state(finished, finished)) {
		daemon->request_job(joined, this);
		block();

		if (current_job != context_stack.top()) { // daemon updated my current job
			current_job->run();

			daemon->erase_job(get_current_job());
		}
	}
	current_job = context_stack.top(); // restore stacked context

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
	printf("Vp %d running the joined job\n", id);

	daemon->erase_job(get_current_job());
	set_current_job(context_stack.top()); // restore stacked context
	context_stack.pop();
	
}

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;
	
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
		daemon->erase_job(joined);
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

uint VirtualProcessor::get_id() const {
	return id;
}

ulong VirtualProcessor::get_job_counter() const {
	return job_counter;
}