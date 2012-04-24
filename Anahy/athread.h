#ifndef ATHREAD_H
#define ATHREAD_H

#include <pthread.h>
#include "Job.h"
#include "JobId.h"

typedef JobId athread_t;
typedef JobAttributes athread_attr_t;


void aInit(int argc, char** argv);
void aTerminate();
void athread_exit(void* value_ptr);
int athread_create(athread_t* thid, athread_attr_t* attr,
	pfunc function, void* args);
int athread_join(athread_t thid, void** result);

#endif