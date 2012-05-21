#include "Daemon.h"
#include "VPEvent.h"
#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

uint Daemon::instance_counter = 0;

/* PRIVATE METHODS */

void* Daemon::call_daemon_run(void* daemon_obj) { // static
	Daemon* daemon = (Daemon*) daemon_obj;
	daemon->run();
	return NULL;
}

void Daemon::run() {
	VPEvent* event;
	VirtualProcessor* vp;

	pthread_mutex_lock(&queue_mutex);
	while(true) {
		//printf("%d processors waiting...\n", processors_waiting);
		if (anahy_stop && (processors_waiting == num_processors)) {
			// anahy signaled to stop and all my processors are waiting
			list<VPEvent*>::iterator it;
			for (it = suspended_events.begin(); it != suspended_events.end(); it++) {
				vp = (*it)->get_sender();
				vp->set_current_job(NULL);
				vp->resume();
				processors_waiting -= 1;
			}
			pthread_mutex_unlock(&queue_mutex);
			return;
		}
		else {
			while (!event_queue.empty()) {
				event = event_queue.front();
				event_queue.pop();
				pthread_mutex_unlock(&queue_mutex);

				handle_vp_event(event);

				pthread_mutex_lock(&queue_mutex);
			}
			pthread_cond_wait(&event_condition, &queue_mutex);
		}
	}
}

// this private method is called in a VP thread
void Daemon::push_vp_event(VPEvent* event) {
	pthread_mutex_lock(&queue_mutex);
	event_queue.push(event);
	if(event->get_type() == GetJob) {
		processors_waiting += 1;
	}
	pthread_cond_signal(&event_condition);
	pthread_mutex_unlock(&queue_mutex);
}

// executes a scheduling operation and return a flag
// indicating success (true) or failure (false)
void Daemon::handle_vp_event(VPEvent* op) {

	// EVERY TIME WE HAVE TO RESUME A VP, processors_waiting -= 1 !!!

	switch (op->get_type()) {
		case NewJob:
			// insert associated job in Anahy graph
			// if there are any processors waiting
			// test and set job's state from ready to running
			// if job was ready, set as current_job of
			// the vp that waited more, processors_waiting--, and resume it
			break;
		case EndOfJob:
			// change job state to finished
			// if there are any processors waiting and
			//  one of them tried a GetJob from associated job
			// then processors_waiting-- and resume it
			break;
		case GetJob:
			// search for a ready using the associated job
			// as parameter...
			// if returned NULL, push this event onto suspended_events.
			// else set sender->current_job and processors_waiting--
			break;
		case DestroyJob:
			// remove job from graph ...
		default:
			puts("Unknown event...");
			exit(EXIT_FAILURE);
	}
}


/* PUBLIC METHODS */

Daemon::Daemon(uint _num_processors) : num_processors(_num_processors) {
	anahy_stop = false;
	id = instance_counter++;
	processors_waiting = 0;

	// get the pointer to the VM object
	anahy = AnahyVM::get_instance_handler();

	// init queue's mutex and condition variable
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_cond_init(&event_condition, NULL);

	// create vp objects
	for (uint i = 0; i < num_processors; ++i) {
		processors.push_back(new VirtualProcessor(this));
	}
}

Daemon::~Daemon() {
	list<VirtualProcessor*>::iterator it;
	for (it = processors.begin(); it != processors.end(); it++) {
		delete *it;
	}
	
	pthread_mutex_destroy(&queue_mutex);
	pthread_cond_destroy(&event_condition);
	instance_counter--;
}

// message received from AnahyVM
void Daemon::start() {
	pthread_create(&thread, NULL, call_daemon_run, (void*) this);
	list<VirtualProcessor*>::iterator it;
	for (it = processors.begin(); it != processors.end(); it++) {
		(*it)->start(); // start my processors
	}
}

// message received from AnahyVM, from MAIN thread
void Daemon::stop() {
	pthread_mutex_lock(&queue_mutex);
	anahy_stop = true;
	pthread_cond_signal(&event_condition);
	pthread_mutex_unlock(&queue_mutex);

	list<VirtualProcessor*>::iterator it;
	for (it = processors.begin(); it != processors.end(); it++) {
		(*it)->stop(); // stop my processors
	}
	pthread_join(thread, NULL);
}

// messages to be received from a VP
void Daemon::new_job(VirtualProcessor* sender, Job* job) {
	push_vp_event(new VPEvent(NewJob, job, sender));
}

void Daemon::get_job(VirtualProcessor* sender, Job* job){
	push_vp_event(new VPEvent(GetJob, job, sender));
	sender->block();
}

void Daemon::end_of_job(VirtualProcessor* sender, Job* job) {
	push_vp_event(new VPEvent(EndOfJob, job, sender));
}

void Daemon::destroy_job(VirtualProcessor* sender, Job* job) {
	push_vp_event(new VPEvent(DestroyJob, job, sender));
}