#ifndef ATHREAD_H
#define ATHREAD_H

#include <pthread.h>

typedef JobId athread_t

void aInit(int* _argc, char*** _argv);
void aTerminate();
int athread_create(athread_t* thid, athread_attr_t* attr,
	pfunc function, void* args);
int athread_join(athread_t thid, void** result);

#endif