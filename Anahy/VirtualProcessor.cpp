#include "VirtualProcessor.h"


/**** STATIC MEMBERS' ITIALIZATIONS ****/

uint VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;


/**** PRIVATE METHODS' DEFINITIONS ****/

void* VirtualProcessor::call_vp_run(void* vp_obj) {
	pthread_setspecific(key, vp_obj);
	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;
	vp->run();
	return NULL;
}

// VP main loop
void VirtualProcessor::run() {
	while(true) {
		daemon->get_job(this, NULL); // ask daemon and wait

		if (!current_job) { // daemon set my job as NULL ...
			// I should stop!
			return;
		}

		current_job->run();
		daemon->end_of_job(this, current_job);
		current_job = NULL;
	}
}

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


/**** STATIC METHODS ****/

// dummy functon to fill pthread_key_create(...) requirements
void VirtualProcessor::call_vp_destructor(void *vp_obj) { }

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

/**** PUBLIC METHODS ****/

VirtualProcessor::VirtualProcessor(Daemon* _daemon) : daemon(_daemon) {
	id = instance_counter++;
	job_counter = 0;
	current_job = NULL;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex); // to block myself in the next lock
}

VirtualProcessor::~VirtualProcessor() {
	instance_counter--;
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

// message received from a daemon object and daemon thread
void VirtualProcessor::start() {
	printf("Starting VP %d\n", id);
	pthread_create(&thread, NULL, call_vp_run, (void*)this);
}

// message received from a daemon object and daemon thread
void VirtualProcessor::stop() {
	pthread_join(thread, NULL);
	printf("Stopping VP %d ...\n", id);
}

// message received from a daemon object but from my thread
void VirtualProcessor::block() {
	pthread_mutex_lock(&mutex);
}

// message received from a daemon object and daemon thread
void VirtualProcessor::resume() {
	pthread_mutex_unlock(&mutex);
}

/* messages to be received from athread API */

VirtualProcessor* VirtualProcessor::get_current_vp() { // class method!
	return (VirtualProcessor*) pthread_getspecific(key);
}

JobHandle VirtualProcessor::create_new_job(pfunc function, void* args, JobAttributes attr) {
	JobId job_id(id, job_counter++);
	Job* job = new Job(job_id, current_job, this, attr, function, args);
	daemon->new_job(this, job);
	JobHandle handle;
	handle.pointer = job;
	handle.id = job_id;
}

void* VirtualProcessor::join_job(JobHandle handle) {
	Job* joined = handle.pointer;
	void* result;

	bool done = false;
	do {
		JobState state = joined->compare_and_swap_state(ready, running);

		if (state == running) {
			// if the joined job was already running ...
			// try to help its execution
			suspend_current_job_and_try_to_help(joined);
		}
		else {
			// if the job was ready for execution or already finished
			if (state == ready) {
				suspend_current_job_and_run_another(joined);
			}
			
			result = joined->get_retval();

			/* if (joined->dec_join_counter() == 0) {
				delete job;
			} */

			done = true;
		}
	} while (!done);
	
	daemon->destroy_job(this, joined);
	return result;
}

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
