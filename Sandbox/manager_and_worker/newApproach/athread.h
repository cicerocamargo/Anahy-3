#ifndef ATHREAD_H
#define ATHREAD_H

#include "Job.h"
#include "JobId.h"
#include "JobAttributes.h"

typedef JobHandle athread_t;
typedef JobAttributes athread_attr_t;


void aInit(int argc, char** argv);
void aTerminate();
void athread_exit(void* value_ptr);
int athread_create(athread_t* thid, athread_attr_t* attr,
	pfunc function, void* args);
int athread_join(athread_t thid, void** result);

//we'll define from now the methods to the job's attributes
int athread_attr_init(athread_attr_t* attr);
int athread_attr_destroy(athread_attr_t* attr);

/*This attribute sets that threads (when terminated) 
 *can or cannot to free its resources.
 ***JOINABLE: Exit status and thread are preserved after 
 the thread terminates to be recovered by pthread_join.
 ***DETACHED: Exit status and thread aren't preserved. 
 When it happens, the thread resources are freed to be 
 able to allocate more threads.
*/
int athread_attr_setdetached(athread_attr_t* attr, JobAttributes_State detach_state);
int athread_attr_getdetached(athread_attr_t* attr, JobAttributes_State* detach_state);

//This attributes indicates the joins' number the thread can receive
int athread_attr_setjoinnumber(athread_attr_t* attr, int _num_joins);
int athread_attr_getjoinnumber(athread_attr_t* attr, int* _num_joins);

/*This attibutes sets the thread's cost, it can be used as support to the
 *scheduling function or to scheduling frequency
*/
int athread_attr_setjobcost(athread_attr_t* attr, int _job_cost);
int athread_attr_getjobcost(athread_attr_t* attr, int* _job_cost);

#endif