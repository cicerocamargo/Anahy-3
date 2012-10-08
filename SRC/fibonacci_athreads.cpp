//#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "AnahyVM.h"

int __count = 0;

using namespace std;

void* par_fib(void* args) {
	long* n = (long*)args;

	if (*n >= 2) {
		long n_minus_1 = *n-1;
		long n_minus_2 = *n-2;

		AnahyJob job_left(par_fib, &n_minus_1, NULL);
		AnahyJob job_right(par_fib, &n_minus_2, NULL);

		AnahyVM::fork(&job_left);
		AnahyVM::fork(&job_right);

		//fib(15); // para aumentar o custo do thread

		AnahyVM::join(&job_right, NULL);
		AnahyVM::join(&job_left, NULL);

		*n = n_minus_1 + n_minus_2;
	}	
	return NULL;
}

int main(int argc, char **argv) {

	long n = atol(argv[1]);
	long result = n;

	AnahyVM::init(argc, argv);

	AnahyJob job(par_fib, &result, NULL);
	AnahyVM::fork(&job);
	AnahyVM::join(&job, NULL);

	cout << "fib(" << n << ") = " << result << endl;
	AnahyVM::terminate();
	return 0;
}