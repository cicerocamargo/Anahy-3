#include "VirtualProcessor.h"
#include "Daemon.h"
#include "Job.h"
#include "JobAttributes.h"
#include "JobId.h"
#include <cstdio>
#include <cstdlib>
#include <sched.h>

/**** STATIC MEMBERS' ITIALIZATIONS ****/

pthread_mutex_t VirtualProcessor::mutex;
uint VirtualProcessor::instance_counter = 0;
long VirtualProcessor::tid_counter = 0;
pthread_key_t VirtualProcessor::key;

/* PRIVATE METHODS */

void* VirtualProcessor::call_vp_run(void* arg) {
	associate_vp_with_current_thread(arg);
	VirtualProcessor* vp = (VirtualProcessor*) arg;

	cpu_set_t cpuset;

	// this set the vp affinity
	CPU_ZERO(&cpuset);
	//printf("%u -- %ld -- %d \n", vp->get_id(), vp->get_tid(), (int) pthread_self());

	CPU_SET(vp->get_tid(), (cpu_set_t*) &cpuset);
	if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
		printf("Error in pthread_setaffinity_np!\n");
	}

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
	if (tid_counter == daemon->get_num_cpus())
		tid_counter = 0;
	
	tid = tid_counter++;
	//printf("RA %ld - %u\n", tid, instance_counter);
	id = instance_counter++;

	local_graph = new JobGraph();

	current_job = NULL;
	job_counter = 0;

	pthread_mutex_init(&mutex, NULL);
}

// called from Daemon Thread
VirtualProcessor::~VirtualProcessor()  {
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
}

//maybe we will use these two methods, but I don't know when and why
void VirtualProcessor::block() {
	pthread_mutex_lock(&mutex);
}

void VirtualProcessor::resume() {
	pthread_mutex_unlock(&mutex);
}

// this is the main loop of vp
void VirtualProcessor::run() {
	//printf("VP %d: Running...\n", id);
	while(true) {
		current_job = request_job(NULL, false);

		if(!current_job) {
			current_job = daemon->work_stealing_function(this);
		}
		if (!current_job) {
			//printf("VP %d: I have no job, I'll stop\n", id);
			break;
		}
		else {
			current_job->run();
			//daemon->erase_job(current_job, this);
		}
	}
}

// dummy functon to fill pthread_key_create(...) requirements
void VirtualProcessor::call_vp_destructor(void *vp_obj) { }

void VirtualProcessor::associate_vp_with_current_thread(void* vp_obj) {
	//printf("VP ASSOCIATING_VP\n");
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

	Job* job = new Job(job_id, current_job, this, attr, function, args);
	
	post_job(job);
	
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
	//printf("VP %d: I'll help the job to run\n", id);

	request_job(joined, false);

	if (current_job != context_stack.top()) { // daemon updated my current job
		current_job->run();

	}

	current_job = context_stack.top(); // restore stacked context

	// here, if daemon didn't change the current job and resumed me
	// (so I got here without executing the IF statement)
	// means that 'joined' got finished before a ready job was available
	
	context_stack.pop();
}


// run another job keeping track of the suspended job
void VirtualProcessor::suspend_current_job_and_run_another(Job* another) {

	context_stack.push(current_job); // save context
	current_job = another; // update current_job

	current_job->run();

	current_job = context_stack.top(); // restore stacked context
	context_stack.pop();
	//printf("Vp %d: A joined job was ran.", id);
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

	if(joined->dec_join_counter()) {
		daemon->erase_job(joined, this);
	}
	return joined->get_retval();
}

void VirtualProcessor::post_job(Job* job) {
	pthread_mutex_lock(&mutex);

	local_graph->insert(job);
	daemon->wake_up_some_waiting_vp();

	pthread_mutex_unlock(&mutex);
}

Job* VirtualProcessor::request_job(Job* _starting_job, bool steal_job) {
	pthread_mutex_lock(&mutex);

	Job* job = NULL;

	job = local_graph->find_a_ready_job(_starting_job, steal_job);
	
	pthread_mutex_unlock(&mutex);

	return job;
}

void VirtualProcessor::erase_job(Job* joined_job) {
	pthread_mutex_lock(&mutex);

	local_graph->erase(joined_job);
	current_job = NULL;

	pthread_mutex_unlock(&mutex);
}

// called from Daemon Thread
void VirtualProcessor::start() {

	pthread_create(&thread, NULL, call_vp_run, this);
}

// called from Daemon Thread
void VirtualProcessor::stop() {
	pthread_join(thread, NULL);
}