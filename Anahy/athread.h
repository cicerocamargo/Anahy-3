#ifndef ATHREAD_H
#define ATHREAD_H

#include "Job.h"
#include "JobId.h"
#include "JobAttributes.h"

typedef JobId athread_t;
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
int athread_attr_setdetached(athread_attr_t* attr, int detach_state);
int athread_attr_getdetached(athread_attr_t* attr, int* detach_state);
int athread_attr_setjoinnumber(athread_attr_t* attr, int number_joins);
int athread_attr_getjoinnumber(athread_attr_t* attr, int* number_joins);
int athread_attr_setjobcost(athread_attr_t* attr, int cost);

#endif