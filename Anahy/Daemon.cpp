#include "Daemon.h"
#include "SchedulingOperation.h"
#include "AnahyVM.h"
#include <cstdio>
#include <cstdlib>

/* PUBLIC METHODS' DEFINITIONS */
Daemon::Daemon() {
	anahy = AnahyVM::get_instance_handler();
}

Daemon::~Daemon() {
	
}

bool execute_operation(SchedulingOperation* op) {
	switch (op->get_type()) {
		case NewJob:
			// do stuff
			return true;
		case EndJob:
			// do stuff
			return true;
		case GetJob:
			Job* job = anahy->find_a_ready_job(op->get_associated_job());
			return job ? true : false;
		case GraphCompleted:
			graph_completed = true;
			// do stuff
			return true;
		default:
			puts("Invalid scheduling operation...");
			exit(EXIT_FAILURE);
	}
}

void Daemon::start() {
	puts("Starting Daemon...");
	
	SchedulingOperation* operation;
	graph_completed = false;
	do {
		pthread_mutex_lock(&queue_mutex);

		if (pending_operations.size() > 0) {
			operation = pending_operations.front();
			pending_operations.pop();	
		}
		else {
			operation = NULL;
			pthread_cond_wait(&new_sched_op, &queue_mutex);
		}

		pthread_mutex_unlock(&queue_mutex);
		
		if (operation) {
			operation_succeeded = execute_operation(operation);
			if (!operation_succeeded) {
				suspended_operations.push_back(operation);
			}
		}

	} while (!graph_completed);
}

void Daemon::stop() {
	puts("Stopping Daemon...");
}

void Daemon::flush() {
	
}

void Daemon::push_scheduling_operation(SchedulingOperation* sched_op) {
	pthread_mutex_lock(&queue_mutex);
	pending_operations.push(sched_op);
	pthread_mutex_unlock(&queue_mutex);
}
