#ifndef ANAHYVM_H
#define ANAHYVM_H

#define VERSION 3.0

#include <pthread.h>
#include <unistd.h>

#include "definitions.h"
#include "AnahyJobAttributes.h"
#include "AnahyJob.h"

using namespace std;

class VirtualProcessor;

class AnahyVM {
	
	static VirtualProcessor* main_vp;
	static int num_cpus;
	static int num_vps;

	static VirtualProcessor** vp_array;
	static pthread_t* thread_array;
	static pthread_key_t key;

	AnahyVM();
	~AnahyVM();

	// for pthread (set/get)specific API
	static void* call_vp_run(void* vp_obj);
	static void call_vp_destructor(void* vp_obj);

	// init helpers
	static void parse_user_options(int argc, char **argv);
	static void show_help();

public:

	static int get_num_cpus() { return num_cpus; }
	static int get_num_vps() { return num_vps; }

	static void init(int argc, char **argv);
	static void terminate();
	static void fork(AnahyJob* job);
	static void join(AnahyJob* job, void** result);

	static void _exit(void* value_ptr);

	// USER ATTRIBUTES INTERFACE

	static void attr_init(AnahyJobAttributes* attr);
	static void attr_setjobcost(AnahyJobAttributes* attr, JobCost cost);
	static void attr_setjobjoins(AnahyJobAttributes* attr, int joins);
	static int attr_getjobcost(AnahyJobAttributes* attr);
};

#endif