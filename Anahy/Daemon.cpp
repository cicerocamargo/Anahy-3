#include "Daemon.h"
#include "SchedulingOperation.h"
#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

uint Daemon::instance_counter = 0;

/* PRIVATE METHODS */

// funtion to be used as start routine for a pthread
// receives a (void*) pointer to a VP and start it
void* Daemon::run_vp(void* vp_obj) { // static

	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;

	// set the current pthread key as a vp pointer
	pthread_setspecific(VirtualProcessor::get_pthread_key(), (void*)vp);
	
	vp->start();
	return NULL;
}

// executes a scheduling operation and return a flag
// indicating success (true) or failure (false)
void Daemon::execute_operation(SchedulingOperation* op) {
	switch (op->get_type()) {
		case NewJob:
			anahy->insert_job(op->get_associated_job());		
			break;
		case EndJob:

			// do stuff
		
			break;
		case GetJob:
			
			// do stuff

			break;
		case GraphCompleted:
					
			// do stuff
			
			break;
		default:
			puts("Invalid scheduling operation...");
			exit(EXIT_FAILURE);
	}
}


/* PUBLIC METHODS' DEFINITIONS */
Daemon::Daemon(uint _num_processors) : num_processors(_num_processors) {
	anahy_is_running = true;
	id = instance_counter++;

	// get the pointer to the VM object
	anahy = AnahyVM::get_instance_handler();

	// init queue's mutex and condition variable
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_cond_init(&new_sched_op, NULL);


	// allocate structures
	vp_array = (VirtualProcessor**) malloc(num_processors*sizeof(VirtualProcessor*));
	vp_threads_array = (pthread_t*) malloc(num_processors*sizeof(pthread_t));

	// create vp objects
	for (uint i = 0; i < num_processors; ++i) {
		vp_array[i] = new VirtualProcessor(this);
	}

	// launch vp objects
	for (uint i = 0; i < num_processors; ++i) {
		if (this->id == 0 && i == 0) {
			// the main VP has id 0 and is associated to Daemon 0
			// DO NOT create a new thread for the main VP !!
			continue;
		}
		pthread_create(&vp_threads_array[i], NULL, run_vp, vp_array[i]);
	}
}

Daemon::~Daemon() {
	for (uint i = 0; i < num_processors; ++i) {
		delete vp_array[i];
	}
	free(vp_array);

	for (uint i = 0; i < num_processors; ++i) {
		if (this->id == 0 && i == 0) {
			// the main VP has id 0 and is associated to Daemon 0
			// DO NOT create a new thread for the main VP !!
			continue;
		}
		pthread_join(vp_threads_array[i], NULL);
	}
	free(vp_threads_array);

	pthread_mutex_destroy(&queue_mutex);
	pthread_cond_destroy(&new_sched_op);
}

void Daemon::start() {
	puts("Starting Daemon...");
	
	SchedulingOperation* operation;
	
	do {
		pthread_mutex_lock(&queue_mutex);

		if (pending_operations.empty()) {
			operation = NULL;
			pthread_cond_wait(&new_sched_op, &queue_mutex);
		}
		else {
			operation = pending_operations.front();
			pending_operations.pop();	
		}

		pthread_mutex_unlock(&queue_mutex);
		
		execute_operation(operation);

	} while (anahy_is_running);
}

void Daemon::stop() {
	puts("Stopping Daemon...");
}

void Daemon::flush() {
	// ??
}

// message to be received from a VP
void Daemon::push_scheduling_operation(SchedulingOperation* sched_op) {
	pthread_mutex_lock(&queue_mutex);
	pending_operations.push(sched_op);
	pthread_cond_signal(&new_sched_op);
	pthread_mutex_unlock(&queue_mutex);
}
