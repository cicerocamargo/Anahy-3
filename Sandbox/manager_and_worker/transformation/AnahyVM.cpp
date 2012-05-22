#include "AnahyVM.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "JobGraph.h"
#include <cstdio>
#include <cstdlib>

list<Daemon*> AnahyVM::daemons;
pthread_mutex_t AnahyVM::mutex;
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

	for (it = daemons.begin(); it != daemons.end(); ++it) {
		(*it)->signal_stop();
	}

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

void AnahyVM::init(int num_daemons, int vps_per_daemon) {
	VirtualProcessor::init_pthread_key();

	for (int i = 0; i < num_daemons; ++i) {
		// create daemon objects
		daemons.push_back(new Daemon(vps_per_daemon));
	}

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
void AnahyVM::blocking_get_job(Daemon* sender) {
	Job* j = get_job(NULL);
	Daemon* d;

	if (!j) {
		daemons_waiting.push(sender);

		if (daemons_waiting.size() == num_daemons) {
			// time to everyone die :)
			while (!daemons_waiting.empty()) {
				d = daemons_waiting.front();
				daemons_waiting.pop();
				sender->set_temp_job(NULL);
				sender->resume();
			}
			return;
		}

		sender->block();
	}
	else {
		sender->set_temp_job(j);	
	}
}

Job* AnahyVM::get_job(Job* joined_job) {
	Job* j;
	pthread_mutex_lock(&mutex);

	j = graph.find_a_ready_job(joined_job);
		
	pthread_mutex_unlock(&mutex);
	return j;
}

void AnahyVM::post_job(Job* new_job, bool scheduled) {
	pthread_mutex_lock(&mutex);

	graph.insert(new_job);
	if (!scheduled && !daemons_waiting.empty()) {
		Daemon* d = daemons_waiting.front();
		d->set_job_and_resume();
	}

	pthread_mutex_unlock(&mutex);
}

void AnahyVM::erase_job(Job* joined_job) {
	pthread_mutex_lock(&mutex);

	graph.erase(joined_job);

	pthread_mutex_unlock(&mutex);
}