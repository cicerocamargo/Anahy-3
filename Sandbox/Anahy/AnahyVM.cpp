#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include "JobGraph.h"
#include <cstdio>
#include <cstdlib>

/* STATIC MEMBERS */

uint AnahyVM::num_daemons = 0;
list<Daemon*> AnahyVM::daemons;
uint AnahyVM::daemons_waiting = 0;
pthread_mutex_t AnahyVM::mutex;
pthread_cond_t AnahyVM::cond;
JobGraph AnahyVM::graph;
VirtualProcessor* AnahyVM::main_vp = 0;

/* PRIVATE METHODS */

void AnahyVM::start_vm() {
	list<Daemon*>::iterator it;

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		(*it)->start(); // start daemons
	}
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set
								// by daemon 0
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	pthread_mutex_unlock(&mutex);
	printf("Init done\n");
}

void AnahyVM::stop_vm() {
	list<Daemon*>::iterator it;

	main_vp->run(); // this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		(*it)->stop(); // stop daemons (who stop their VPs)
	}
}


/* PUBLIC METHODS */

// messages received from the API

void AnahyVM::init(int _num_daemons, int vps_per_daemon) {
	num_daemons = _num_daemons;
	VirtualProcessor::init_pthread_key();

	for (int i = 0; i < num_daemons; ++i) {
		// create daemon objects
		daemons.push_back(new Daemon(vps_per_daemon));
	}

	//pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex); // since the main thread has the VM's lock,
								// it can block itself in the next call
								// to wait for the VP 0 to be associated
								// with the main thread

	start_vm();
}

void AnahyVM::terminate() {
	stop_vm();
	
	list<Daemon*>::iterator it;

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		delete *it; // destroy daemon
	}
	daemons.clear();

	//pthread_cond_destroy(&cond);
	//pthread_mutex_destroy(&mutex);
	VirtualProcessor::delete_pthread_key();
}


void AnahyVM::create(JobHandle* handle, JobAttributes* attr,
	pfunc function, void* args) {

	VirtualProcessor* vp = VirtualProcessor::get_current_vp();
	*handle = vp->create_new_job(function, args, attr);
	
}

void AnahyVM::join(JobHandle handle, void** result) {
	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	void* temp = vp->join_job(handle);

	if(result) {
		*result = temp;
	}
}

// messages received from a daemon threads

// Daemon 0 call this method to signal the main
// thread that VP 0 is already associated with
// the main thread (with //pthread_setspecific)
void AnahyVM::set_main_vp(VirtualProcessor* vp) {
	main_vp = vp;
	pthread_mutex_unlock(&mutex);
}

// When every VP is blocked on a GetJob,
// Daemon sends this message to wait for a job.
Job* AnahyVM::get_job(VPEvent event) {
	Job* job = NULL;
	//pthread_mutex_lock(&mutex);
	
	job = graph.find_a_ready_job(event.get_job());

	//pthread_mutex_unlock(&mutex);
	return job;
}

Job* AnahyVM::blocking_get_job(Daemon* sender) {
	Job* job;
	//pthread_mutex_lock(&mutex);

	job = graph.find_a_ready_job(NULL);
	
	if (!job) {
				
		daemons_waiting++;
		if (daemons_waiting == num_daemons) {
			list<Daemon*>::iterator it;
			for (it = daemons.begin(); it != daemons.end(); ++it) {
				(*it)->set_should_stop();
			}
			daemons_waiting = 0;
			//pthread_cond_broadcast(&cond);
			
		}
		else {
			//pthread_cond_wait(&cond, &mutex);
			// someone pushed on the event queue OR
			// changed the should_stop var of the
			// Daemon who called this
		}
	}

	//pthread_mutex_unlock(&mutex);

	return job;
}

// scheduled indicates if the job was already destined
// to a VP before daemon posted it in the graph
void AnahyVM::post_job(VPEvent event, bool scheduled) {
	//pthread_mutex_lock(&mutex);

	graph.insert(event.get_job());
	if (!scheduled && daemons_waiting) {
		// send this event to another daemon waiting
		forward_to_other_daemons(event);
		//pthread_cond_broadcast(&cond);
	}

	//pthread_mutex_unlock(&mutex);
}

void AnahyVM::erase_job(Job* joined_job) {
	//pthread_mutex_lock(&mutex);

	graph.erase(joined_job);

	//pthread_mutex_unlock(&mutex);
}


void AnahyVM::forward_to_other_daemons(VPEvent event){
	event.set_fwd_true();
	list<Daemon*>::iterator it;
	for (it = daemons.begin(); it != daemons.end(); ++it) {
		if (*it != event.get_origin()) {
			(*it)->push_event(event);
		}
	}
	daemons_waiting = 0;
}

void AnahyVM::forward_end_of_job(VPEvent event) {
	//pthread_mutex_lock(&mutex);
	forward_to_other_daemons(event);
	//pthread_cond_broadcast(&cond);
	//pthread_mutex_unlock(&mutex);
}
