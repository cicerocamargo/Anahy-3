#include "VirtualProcessor.h"
#include "SchedulingOperation.h"


/**** STATIC MEMBERS' ITIALIZATIONS ****/

uint VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;


/**** PRIVATE METHODS' DEFINITIONS ****/

void VirtualProcessor::notify_finished_job_to_daemon(Job* job) {
	SchedulingOperation* op = new SchedulingOperation(EndJob, job, this);
    daemon->push_scheduling_operation(op);
}

void VirtualProcessor::notify_new_job_to_daemon(Job* job) {
    SchedulingOperation* op = new SchedulingOperation(NewJob, job, this);
    daemon->push_scheduling_operation(op);
}

Job* VirtualProcessor::ask_daemon_for_new_job(Job* job) {
	// create a new scheduling operation and push onto daemon's queue
	SchedulingOperation* op = new SchedulingOperation(GetJob, job, this);
    daemon->push_scheduling_operation(op);

    // wait for the operation to be finished
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&operation_finished, &mutex);
    pthread_mutex_unlock(&mutex);

    // now job points to nil or a ready job
    return job;
}

/**** STATIC METHODS ****/

// dummy functon to fill pthread_key_create(...) requirements
void VirtualProcessor::call_vp_destructor(void *vp_obj) { }

void VirtualProcessor::init_pthread_key() {
	pthread_key_create(&key, call_vp_destructor);
}

pthread_key_t VirtualProcessor::get_pthread_key() {
	return key;
}

void VirtualProcessor::delete_pthread_key() {
	pthread_key_delete(key);
}

/**** PUBLIC METHODS ****/

VirtualProcessor::VirtualProcessor(Daemon* _daemon) {
	id = instance_counter++;
	anahy_is_running = true;
	daemon = _daemon;
	job_counter = 0;
	current_job = NULL;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&operation_finished, NULL);
}

VirtualProcessor::~VirtualProcessor() {
	
}


void VirtualProcessor::start() {
	printf("VP %d running on thread %lu\n", id, (ulong)pthread_self());
	do {
		current_job = ask_daemon_for_new_job(NULL);
		if (current_job) {
			current_job->run();
			notify_finished_job_to_daemon(current_job);
			current_job = NULL;
		}
	} while (anahy_is_running);
}

void VirtualProcessor::stop() {
	// compare_and_swap(&program_end, false, true);
	// wake vp thread to break the while loop in start() method;
}

void VirtualProcessor::flush() {
	// ???
}


/* messages to be received from athread API */

JobId VirtualProcessor::create_new_job(pfunc function, void* args, JobAttributes attr) {
	JobId jid(id, job_counter++);
	Job* job = new Job(jid, current_job, this, attr, function, args);
	notify_new_job_to_daemon(job);
	return jid;
}

void VirtualProcessor::suspend_current_job_and_try_to_help(Job* job) {
	Job* temp = ask_daemon_for_new_job(job);
	if (temp) {
		suspend_current_job_and_run_another(temp);
	}
}


// run another job keeping track of the suspended job
void VirtualProcessor::suspend_current_job_and_run_another(Job* job) {
	suspended_jobs.push(current_job); // memorize current job
	
	current_job = job;
	job->run();
	notify_finished_job_to_daemon(job);
	
	// restore previous job
	current_job = suspended_jobs.front();
	suspended_jobs.pop();
}

/* msg to be received from a Daemon */
void VirtualProcessor::continue_execution() {
	pthread_cond_signal(&operation_finished);
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
