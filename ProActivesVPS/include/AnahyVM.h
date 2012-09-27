#ifndef ANAHYVM_H
#define ANAHYVM_H

#define VERSION 3.0

#include <list>
#include <pthread.h>
#include <unistd.h>

#include "Job.h"
#include "JobId.h"

#include "definitions.h"

typedef JobHandle athread_t;
typedef JobAttributes athread_attr_t;

using namespace std;

class VirtualProcessor;
class Job;
class JobAttributes;

class AnahyVM {
	
	static VirtualProcessor* main_vp;
	static int num_cpus;
	static int num_vps;

	static list<VirtualProcessor*> vps;

	AnahyVM();
	~AnahyVM();

	static void start_vps();
	static void stop_vps();

	static void help();

public:

	inline static int get_num_cpus() { return num_cpus; }
	inline static int get_num_vps() { return num_vps; }

	static void init(int argc, char **argv);
	static void terminate();
	static void _exit(void* value_ptr);
	static void create(athread_t* handle, athread_attr_t* attr, pfunc function, void* args);
	static void join(athread_t handle, void** result);

	// USER ATTRIBUTES INTERFACE

	static void attr_init(JobAttributes* attr);

	static void attr_setjobcost(JobAttributes* attr, JobCost cost);
	static void attr_setjobjoins(JobAttributes* attr, int joins);

	static int attr_getjobcost(JobAttributes* attr);
};

#endif