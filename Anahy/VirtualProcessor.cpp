#include "VirtualProcessor.h"
#include "Daemon.h"
#include "SchedulingOperation.h"
#include "definitions.h"

/* STATIC MEMBERS' ITIALIZATIONS */
uint VirtualProcessor::instance_counter = 0;
pthread_key_t VirtualProcessor::key;

void VirtualProcessor::call_vp_destructor(void *vp_obj) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;
	//printf("Destroying VP %d\n", vp->id);
	delete vp;
}

/* PRIVATE METHODS' DEFINITIONS */
void VirtualProcessor::notify_finished_job_to_daemon(Job* job) {
	SchedulingOperation* op = new SchedulingOperation(EndJob, job, this);
    daemon->push_scheduling_operation(op);
}

void VirtualProcessor::notify_new_job_to_dameon(Job* job) {
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

/* PUBLIC METHODS' DEFINITIONS */
VirtualProcessor::VirtualProcessor(Daemon* _daemon, pthread_t _thread) {
	daemon = _daemon;
	thread = _thread;
	id = instance_counter++;
	job_counter = 0;
	program_end = false;
	//operation_finished = PTHREAD_COND_INITIALIZER		?????
	//mutex = PTHREAD_MUTEX_INITIALIZER;				?????
}

VirtualProcessor::~VirtualProcessor() {
	
}

void VirtualProcessor::start() {
	while (true) {
		if (program_end) {
			break;
		}

		current_job = ask_daemon_for_new_job(NULL);
		if (current_job) {
			current_job->run();
			notify_finished_job_to_daemon(current_job);
		}
	}
}

void VirtualProcessor::stop() {
	
}

void VirtualProcessor::flush() {
	
}


/* messages to be received from athread API */

JobId create_new_job(pfunc function, void* args, JobAttributes attr) {

	JobId jid(id, job_counter++);
	Job* job = new Job(jid, current_job, this, attr, function, args);
	notify_new_job_to_dameon(job);
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
	Job* previous_job = current_job;
	current_job = job;
	job->run();
	notify_finished_job_to_daemon(job);
	current_job = previous_job;
}

/* msg to be received from a Daemon */
void VirtualProcessor::signal_operation_finished() {
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

pthread_t VirtualProcessor::get_thread() const {
	return thread;
}

pthread_mutex_t* VirtualProcessor::get_mutex() {
	return &mutex;
}

pthread_key_t VirtualProcessor::get_pthread_key() {
		return key;
}