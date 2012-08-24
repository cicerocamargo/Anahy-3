#ifndef ANAHYVM_H
#define ANAHYVM_H

#include <list>
#include <pthread.h>

#include "Job.h"
#include "JobId.h"
#include "JobAttributes.h"

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
	static void create(JobHandle* handle, JobAttributes* attr,
	pfunc function, void* args);
	static void join(JobHandle handle, void** result);

	static int get_num_cpus() { return num_cpus; }

	//we'll define from now the methods to the job's attributes
	int attr_init(JobAttributes* attr);
	int attr_destroy(JobAttributes* attr);

/*This attribute sets that threads (when terminated) 
 *can or cannot to free its resources.
 ***JOINABLE: Exit status and thread are preserved after 
 the thread terminates to be recovered by pthread_join.
 ***DETACHED: Exit status and thread aren't preserved. 
 When it happens, the thread resources are freed to be 
 able to allocate more threads.
*/
	int attr_setdetached(JobAttributes* attr, JobAttributes_State detach_state);
	int attr_getdetached(JobAttributes* attr, JobAttributes_State* detach_state);

	//This attributes indicates the joins' number the thread can receive
	int attr_setjoinnumber(JobAttributes* attr, int _num_joins);
	int attr_getjoinnumber(JobAttributes* attr, int* _num_joins);

	/*This attibutes sets the thread's cost, it can be used as support to the
 	* scheduling function or to scheduling frequency
	*/
	int attr_setjobcost(JobAttributes* attr, int _job_cost);
	int attr_getjobcost(JobAttributes* attr, int* _job_cost);
};

#endif