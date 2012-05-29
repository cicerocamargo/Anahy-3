#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include "AnahyVM.h"
#include "JobAttributes.h"

using namespace std;

fstream log;



void* par_fib(void* args) {
	long* _n = ((long*) args);
	long n = *(_n);
	long res;

	if (n < 2) {
		log << "Running fib " << n << "\n";
		res = n;
	}
	else {
		log << "evaluating fib " << n << "\n";
		
		JobHandle m1, m2;
		JobAttributes attr;
		attr.set_initialized(true);
		attr.set_num_joins(1);
		AnahyVM::create(&m1, &attr, par_fib, (void*) new long(n-1));
		AnahyVM::create(&m2, NULL, par_fib, (void*) new long(n-2));

		log << "\tcreated fib " << n-1 << "\n";
		log << "\tcreated fib " << n-2 << "\n";

		long* fib_m1 = new long();
		long* fib_m2 = new long();
		AnahyVM::join(m1, (void**) &fib_m1);
		AnahyVM::join(m2, (void**) &fib_m2);
		res = *fib_m1 + *fib_m2;

		delete fib_m1;
		delete fib_m2;
	}	

	delete _n;
	return (void*) new long(res);
}

int main(int argc, char const *argv[])
{
	log.open("main.log", fstream::out);
	srand(time(NULL));

	int daemons = atoi(argv[1]);
	int vps_per_daemon = atoi(argv[2]);
	long n = atol(argv[3]);

	AnahyVM::init(daemons, vps_per_daemon);
	
	JobHandle handle;
	AnahyVM::create(&handle, NULL, par_fib, (void*) new long(n));
	long* result = new long(0);
	AnahyVM::join(handle, (void**) &result);

	AnahyVM::terminate();	
	log.close();

	cout << endl << (*result) << endl;
	delete result;

	//cout << fib_(atoi(argv[3])) << endl;

	return 0;
}