#include "athread.h"
#include <cstdio> 

void* func(void* args) {
	printf("toto\n");
	athread_exit(NULL);
}

int main (int argc, char *argv[]) {
	aInit(argc, argv);
	athread_t thid;
	athread_create(&thid, NULL, func, NULL);
	athread_join(thid, NULL);
	aTerminate();
	return 0;
}