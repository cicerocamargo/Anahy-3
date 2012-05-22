#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "AnahyVM.h"

using namespace std;

fstream log;

int fib_(int n) {
	return n < 2 ? n : (fib_(n-1) + fib_(n-2));
}

void* run_fib_(void* args) {
	int n = *((int*) args);
	
	if (n > 40) {
		log << "evaluating fib " << n << "\n";
		JobHandle m1, m2;
		AnahyVM::create(&m1, NULL, run_fib_, (void*) new int(n-1));
		log << "\tcreated fib " << n-1 << "\n";
		AnahyVM::create(&m2, NULL, run_fib_, (void*) new int(n-2));
		log << "\tcreated fib " << n-2 << "\n";
	}	
	else {
		log << "Running fib " << n << "\n";
		fib_(n);
	}
}

int main(int argc, char const *argv[])
{
	log.open("main.log", fstream::out);
	srand(time(NULL));

	int daemons = atoi(argv[1]);
	int vps_per_daemon = atoi(argv[2]);
	int n = atoi(argv[3]);

	AnahyVM::init(daemons, vps_per_daemon);
	
	JobHandle handle;
	AnahyVM::create(&handle, NULL, run_fib_, (void*) new int(n));
	AnahyVM::join(handle, NULL);

	AnahyVM::terminate();	
	log.close();
	return 0;
}