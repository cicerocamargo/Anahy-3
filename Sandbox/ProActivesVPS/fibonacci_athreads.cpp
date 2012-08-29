#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "AnahyVM.h"
#include "JobAttributes.h"

int __count = 0;

using namespace std;

long fib(long n) { return n < 2 ? n : fib(n-1) + fib(n-2); }

void* par_fib(void* args) {
	long* _n = ((long*) args);
	long n = *(_n);
	long res;

	//printf("%d:\tFibo(%lu)\n", __count++, n);
	if (n < 2) {
		res = n;
		//fib(15); // para aumentar o custo do thread
	}
	else {
		JobHandle m1, m2;
		long* n_minus_1 = new long(n-1);
		long* n_minus_2 = new long(n-2);

		AnahyVM::create(&m1, NULL, par_fib, (void*) n_minus_1);
		AnahyVM::create(&m2, NULL, par_fib, (void*) n_minus_2);

		long* fib_m1 = new long();
		long* fib_m2 = new long();
		//fib(15); // para aumentar o custo do thread

		AnahyVM::join(m1, (void**) &fib_m1);
		AnahyVM::join(m2, (void**) &fib_m2);

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
		AnahyVM::init(vps);
		JobHandle handle;
		
		AnahyVM::create(&handle, NULL, par_fib, (void*) new long(n));
		
		//cout << "Create: Done" << endl;

		long* result = new long(0);
		AnahyVM::join(handle, (void**) &result);

		cout << "fib(" << n << ") = " << (*result) << endl;
		delete result;
		AnahyVM::terminate();
	}

	return 0;
}