//#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "../include/AnahyVM.h"

int __count = 0;

using namespace std;

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
		long n_minus_1 = n-1;
		long n_minus_2 = n-2;

		AnahyVM::create(&m1, NULL, par_fib, (void*) &n_minus_1);
		AnahyVM::create(&m2, NULL, par_fib, (void*) &n_minus_2);

		long fib_m1, fib_m2;
		//fib(15); // para aumentar o custo do thread

		AnahyVM::join(m1, (void**) &fib_m1);
		AnahyVM::join(m2, (void**) &fib_m2);

		res = fib_m1 + fib_m2;
	}	

	return (void*)res;
}

int main(int argc, char **argv) {

	long n = atol(argv[1]);
		AnahyVM::init(argc, argv);
		athread_t handle;
		athread_attr_t attr;
		
		AnahyVM::attr_init(&attr);
		AnahyVM::attr_setjobcost(&attr, MINIMUM_COST);

		AnahyVM::create(&handle, NULL, par_fib, (void*) new long(n));
		
		//cout << "Create: Done" << endl;

		long result;
		AnahyVM::join(handle, (void**) &result);

		cout << "fib(" << n << ") = " << result << endl;
		AnahyVM::terminate();
	return 0;
}