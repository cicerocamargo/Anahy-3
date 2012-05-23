#include "AnahyVM.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "JobGraph.h"
#include <cstdio>
#include <cstdlib>

/* STATIC MEMBERS */

uint AnahyVM::num_daemons = 0;
uint AnahyVM::daemons_waiting = 0;
list<Daemon*> AnahyVM::daemons;
pthread_mutex_t AnahyVM::mutex;
pthread_cond_t AnahyVM::cond;
JobGraph AnahyVM::graph;
VirtualProcessor* AnahyVM::main_vp = 0;

/* PRIVATE METHODS */

// int fib(int n) {
// 	return n < 2 ? n : (fib(n-1) + fib(n-2));
// }

// void* run_fib(void* args) {
// 	fib(40);
// 	JobHandle handle;
// 	AnahyVM::create(&handle, NULL, run_fib, NULL);
// 	AnahyVM::join(handle, NULL);
// }

void AnahyVM::start_vm() {
	list<Daemon*>::iterator it;

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		(*it)->start(); // start daemons
	}
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set
								// by daemon 0
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	pthread_mutex_unlock(&mutex);
}

void AnahyVM::stop_vm() {
	list<Daemon*>::iterator it;

	main_vp->run(); // this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		(*it)->stop(); // stop daemons (who stop their VPs)
		printf("Thread of daemon %d joined!\n", (*it)->get_id());
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

	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex); // since the main thread has the VM's lock,
								// it can block itself in the next call
								// to wait for the VP 0 to be associated
								// with the main thread

	// for (int i = 0; i < 20; ++i) {
	// 	// initial work
	// 	JobId job_id(0, i);
	// 	graph.insert(new Job(job_id, NULL, NULL, 0, run_fib, NULL));		
	// }

	start_vm();
}

void AnahyVM::terminate() {
	stop_vm();
	
	list<Daemon*>::iterator it;

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		delete *it; // destroy daemon
	}
	daemons.clear();

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	VirtualProcessor::delete_pthread_key();
}


void AnahyVM::create(JobHandle* handle, JobAttributes* attr,
	pfunc function, void* args) {

	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	if(!vp) {
		puts("erro!");
		exit(1);
	}
	else {
		printf("Create from VP %u\n", vp->get_id());
		*handle = vp->create_new_job(function, args, attr);
	}
}

void AnahyVM::join(JobHandle handle, void** result) {
	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	if(!vp) {
		puts("erro!");
		exit(1);
	}
	else {
		printf("Join from VP %u\n", vp->get_id());
	}
}

// messages received from a daemon threads

// Daemon 0 call this method to signal the main
// thread that VP 0 is already associated with
// the main thread (with pthread_setspecific)
void AnahyVM::set_main_vp(VirtualProcessor* vp) {
	main_vp = vp;
	pthread_mutex_unlock(&mutex);
}

// When every VP is blocked on a GetJob,
// Daemon sends this message to wait for a job.
Job* AnahyVM::blocking_get_job() {
	Job* job = NULL;
	pthread_mutex_lock(&mutex);
	daemons_waiting++;

	while (true) {
		if (daemons_waiting == num_daemons) {
			printf("All daemons_waiting...\n");
			pthread_cond_broadcast(&cond);
			break;	
		}

		job = graph.find_a_ready_job(NULL);
		if(job) {
			printf("Daemon resuming...\n");
			daemons_waiting--;
			break;
		}
		else {	// there's no work but some worker
				// can still generate work
			printf("Daemon Blocked! Daemons_waiting = %d\n", daemons_waiting);
			pthread_cond_wait(&cond, &mutex);
		}
	}

	pthread_mutex_unlock(&mutex);
	return job;
}

Job* AnahyVM::get_job(Job* joined_job) {
	Job* j;
	pthread_mutex_lock(&mutex);

	j = graph.find_a_ready_job(joined_job);
		
	pthread_mutex_unlock(&mutex);
	return j;
}

// scheduled indicates if the job was already destined
// to a VP before daemon posted it in the graph
void AnahyVM::post_job(Job* new_job, bool scheduled) {
	pthread_mutex_lock(&mutex);

	graph.insert(new_job);
	if (!scheduled && daemons_waiting) {
		pthread_cond_signal(&cond);
	}

	pthread_mutex_unlock(&mutex);
}

void AnahyVM::erase_job(Job* joined_job) {
	pthread_mutex_lock(&mutex);

	graph.erase(joined_job);

	pthread_mutex_unlock(&mutex);
}