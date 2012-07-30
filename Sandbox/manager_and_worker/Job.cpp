#include "Work.h"
#include <cstdlib>

Job::Job() {
	amount = rand() % 2 + 1;
}

int fib(int n) {
	return n < 2 ? n : (fib(n-1) + fib(n-2));
}

bool Job::run() {
	fib(37+amount);
	return amount == 2 ? true : false;
}
