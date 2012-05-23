#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include "AnahyVM.h"
#include "JobAttributes.h"

using namespace std;

fstream log;

int fib_(int n) {
	return n < 2 ? n : (fib_(n-1) + fib_(n-2));
}

void* run_fib_(void* args) {
	int* _n = ((int*) args);
	int n = *(_n);
	int res;

	if (n > 40) {
		log << "evaluating fib " << n << "\n";
		
		JobHandle m1, m2;
		JobAttributes attr;
		attr.set_initialized(true);
		attr.set_num_joins(1);
		AnahyVM::create(&m1, &attr, run_fib_, (void*) new int(n-1));
		AnahyVM::create(&m2, NULL, run_fib_, (void*) new int(n-2));

		log << "\tcreated fib " << n-1 << "\n";
		log << "\tcreated fib " << n-2 << "\n";

		int* fib_m1 = new int();
		int* fib_m2 = new int();
		AnahyVM::join(m1, (void**) &fib_m1);
		AnahyVM::join(m2, (void**) &fib_m2);
		res = *fib_m1 + *fib_m2;

		delete fib_m1;
		delete fib_m2;
	}	
	else {
		log << "Running fib " << n << "\n";
		res = fib_(n);
	}

	delete _n;
	return (void*) new int(res);
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
	int* result = new int();
	AnahyVM::join(handle, (void**) &result);

	AnahyVM::terminate();	
	log.close();

	cout << endl << (*result) << endl;
	delete result;

	//cout << fib_(atoi(argv[3])) << endl;

	return 0;
}