#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <list>
#include <pthread.h>

#include "Job.h"
#include "JobId.h"

using namespace std;

class VirtualProcessor;
class Job;

class AnahyVM {
	
	static VirtualProcessor* main_vp;
	static int num_cpus;
	static int num_vps;

	static list<VirtualProcessor*> vps;

	AnahyVM();
	~AnahyVM();

	static void start_vps();
	static void stop_vps();

public:
	static void init(int _num_vps);
	static void terminate();
	static void exit(void* value_ptr);
	static void create(JobHandle* handle, pfunc function, void* args);
	static void join(JobHandle handle, void** result);

	inline static int get_num_cpus() { return num_cpus; }
	inline static int get_num_vps() { return num_vps; }
};

#endif