#include "athread.h"

void* toto(void* args) {
	return NULL;
}

int main (int argc, char** argv) {
	athread_t t;
	aInit(&argc, &argv);
	athread_create(&t, NULL, toto, NULL);
	athread_join(t, NULL);
	aTerminate();
	return 0;
}