#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include "AnahyVM.h"

int fib_(int n) {
	return n < 2 ? n : (fib_(n-1) + fib_(n-2));
}

void* run_fib_(void* args) {
	fib_(40);
}

int main(int argc, char const *argv[])
{
	srand(time(NULL));

	int managers = atoi(argv[1]);
	int workers_per_manager = atoi(argv[2]);

	AnahyVM::init(managers, workers_per_manager);
	
	JobHandle handle;
	AnahyVM::create(&handle, NULL, run_fib_, NULL);
	AnahyVM::join(handle, NULL);

	AnahyVM::terminate();	

	return 0;
}