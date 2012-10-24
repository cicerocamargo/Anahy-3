#include "AnahySmartStack.h"
#include <iostream>
#include <fstream>
#define NUM_TESTS 10
#define MAX_SIZE 100000000
#define PUSH 0
#define POP 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

ofstream logfile;

struct timeval start, stop;
unsigned diffsec, diffusec;

void compute_time_and_flush();

void time_log_start() {
	gettimeofday(&start, NULL);
}

void time_log_stop() {
	gettimeofday(&stop, NULL);
	compute_time_and_flush();
}

void compute_time_and_flush() {
		diffsec = stop.tv_sec - start.tv_sec;
		diffusec = (stop.tv_usec - start.tv_usec) >= 0 ? (stop.tv_usec - start.tv_usec) : 1000000 - stop.tv_usec;
		
		printf("%d.%d seconds\n", diffsec, diffusec);	
}

void switch_operation(int* op) {
	*op = (*op == PUSH ? POP : PUSH);	
}

void print_stack(AnahySmartStack<int>& stack) {
	while (stack.size() > 0) {
		cout << stack.size() << endl;
		logfile << stack.top() << endl;
		stack.pop();
	}
}

int main(int argc, char const *argv[])
{
	AnahySmartStack<int> stack;
	int operation = PUSH;
	int num_operations = MAX_SIZE;

	time_log_start();

	while (num_operations > 1) {
#ifdef DEBUG
		cout << "size: " << stack.size() << "\t>\t" <<  num_operations << (operation == PUSH ? " PUSHs" : " POPs") << endl;
#endif
		for (int i = 0; i < num_operations; ++i)
		{
			if (operation == PUSH) {
				stack.push(i);
			} else {
				stack.pop();
			}
		}

		num_operations /= 2;
		switch_operation(&operation);
	}
	while (num_operations < MAX_SIZE) {
		num_operations += num_operations;
#ifdef DEBUG
		cout << "size: " << stack.size() << "\t>\t" <<  num_operations << (operation == PUSH ? " PUSHs" : " POPs") << endl;
#endif
		for (int i = 0; i < num_operations; ++i)
		{
			if (operation == PUSH) {
				stack.push(i);
			} else {
				stack.pop();
			}
		}
		
		switch_operation(&operation);
	}

	time_log_stop();

	// logfile.open("anahy.log");
	// print_stack(stack);
	// logfile.close();
	return 0;
}