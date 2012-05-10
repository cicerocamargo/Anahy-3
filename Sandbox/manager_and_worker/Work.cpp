#include "Work.h"
#include <cstdlib>

Work::Work() {
	amount = rand() % 2 + 1;
}

int fib(int n) {
	return n < 2 ? n : (fib(n-1) + fib(n-2));
}

bool Work::run() {
	fib(37+amount);
	return amount == 2 ? true : false;
}
