#include "Daemon.h"
#include "AnahyVM.h"
#include <cstdio>

/* PUBLIC METHODS' DEFINITIONS */
Daemon::Daemon() {
	anahy = AnahyVM::get_instance_handler();
}

Daemon::~Daemon() {
	
}

void Daemon::start() {
	puts("Starting Daemon...");
}

void Daemon::stop() {
	puts("Stopping Daemon...");
}

void Daemon::flush() {
	
}

void Daemon::push_scheduling_operation(SchedulingOperation* sched_op) {
	
}


/* getters and setters */
queue<SchedulingOperation*> Daemon::get_new_operations() const {
	return new_operations;
}

list<SchedulingOperation*> Daemon::get_suspended_operations() const {
	return suspended_operations;
}
