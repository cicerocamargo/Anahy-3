#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "Daemon.h"
#include "JobAttributes.h"

using namespace std;

long fib(long n) { return n < 2 ? n : fib(n-1) + fib(n-2); }

void* par_fib(void* args) {
	long* _n = ((long*) args);
	long n = *(_n);
	long res;
	cout << "Fibo(" << n << ")" << endl;
	if (n < 2) {
		res = n;
		//fib(15); // para aumentar o custo do thread
	}
	else {
		JobHandle m1, m2;
		Daemon::create(&m1, NULL, par_fib, (void*) new long(n-1));
		Daemon::create(&m2, NULL, par_fib, (void*) new long(n-2));

		long* fib_m1 = new long();
		long* fib_m2 = new long();
		//fib(15); // para aumentar o custo do thread

		Daemon::join(m1, (void**) &fib_m1);
		Daemon::join(m2, (void**) &fib_m2);

		res = *fib_m1 + *fib_m2;

		delete fib_m1;
		delete fib_m2;
	}	

	delete _n;
	return (void*) new long(res);
}

int main(int argc, char const *argv[]) {

	int vps = atoi(argv[1]);
	long n = atol(argv[2]);

	if (vps == 0) {
		cout << "fib(" << n << ") = " << fib(n) << endl;
	}
	else {
		Daemon::init(vps);
	
		JobHandle handle;
		Daemon::create(&handle, NULL, par_fib, (void*) new long(n));
		long* result = new long(0);
		Daemon::join(handle, (void**) &result);

		Daemon::terminate();

		cout << "fib(" << n << ") = " << (*result) << endl;
		delete result;
	}

	return 0;
}