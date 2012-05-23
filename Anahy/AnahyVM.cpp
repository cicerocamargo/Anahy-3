#include "AnahyVM.h"
#include <cstdio>
#include <cstdlib>

/* STATIC MEMBERS' ITIALIZATIONS */
AnahyVM* AnahyVM::unique_instance = NULL;
JobGraph AnahyVM::graph;

/* PRIVATE METHODS */
AnahyVM::AnahyVM(uint _num_daemons, uint _vps_per_daemon,
	uint _scheduling_function, uint mode_operation) :
	num_daemons(_num_daemons) , vps_per_daemon(_vps_per_daemon)
{
	graph = new JobGraph();

	for (uint i = 0; i < num_daemons; ++i) {
		daemons.push_back(new Daemon(vps_per_daemon));
	}
}

AnahyVM::~AnahyVM() {
	list<Daemon*>::iterator it;
	for (it = daemons.begin(); it != daemons.end(); it++) {
		delete (*it);
	}
	daemons.clear();
}

void AnahyVM::start() {
	list<Daemon*>::iterator it;
	for (it = daemons.begin(); it != daemons.end(); it++) {
		(*it)->start();
	}
}

void AnahyVM::stop() {
	list<Daemon*>::iterator it;
	for (it = daemons.begin(); it != daemons.end(); it++) {
		(*it)->stop();
	}
}

/* PUBLIC METHODS' DEFINITIONS */

AnahyVM* AnahyVM::get_instance_handler() {
	return unique_instance;
}

// messages to be received directly from the API
void AnahyVM::boot(uint num_daemons, uint vps_per_daemon, uint scheduling_function, uint mode_operation) {
	puts("Booting AnahyVM...");

	VirtualProcessor::init_pthread_key(); // before anything
	if (!unique_instance) {
		// create AnahyVM, who creates Daemons, who creates VPs
		unique_instance = new AnahyVM(num_daemons, vps_per_daemon,
			scheduling_function, mode_operation);
	}
	unique_instance->start(); // start VM!
	
	puts("Done!");
}

void AnahyVM::shut_down() {
	puts("Shuting AnahyVM down...");
	unique_instance->stop(); // stop VM ...
	delete unique_instance;
	VirtualProcessor::delete_pthread_key(); // after anything
}

// MESSAGES TO BE RECEIVED FROM A DAEMON

// insert a new job in the graph
void AnahyVM::insert_job(Job* job) {

	graph.insert(job);
}

// removes a job from the graph
void AnahyVM::remove_job(Job* job) {
	graph.erase(job);
}

// uses the scheduling function to find
// the most suitable ready job in the graph
Job* AnahyVM::find_a_ready_job(Job* job) {
	graph.find_a_ready_job(job);
}


void AnahyVM::create_dummy_job(pfunc func, void* args) {
	// JobId id(666,999);
	// Job* parent = NULL;
	// JobAttributes attr = 0;
	// VirtualProcessor* vp = NULL;

	// Job* j = new Job(id, parent, vp, attr, func, args);

	// root_jobs.push_back(j);
	// job_map[id] = j;
}