#include <stdlib.h>
#include <stdio.h>
#include <list>
#include "task.h"
#include "scheduler.h"

typedef unsigned long int ulong;

class SumTask : public Task {
public:
	void run() {
		ulong* sum = new ulong(0);
		for (it = predecessors.begin(); it != predecessors.end(); it++) {
			ulong* temp = (ulong*)(*it)->get_result();
			*sum += (*temp);
		}
		if (successors.empty()) { // this is the output task
			printf("Result is %lu\n", *sum);
		}
		result = (void*)sum;
	}
};

class FibTask : public Task {
	ulong n;
public:
	FibTask(ulong _n) : n(_n) {}
	
	void generate_DAG(SumTask* sum) {
		if (n < 2) {
			add_successor(sum);
		}
		else {
			FibTask* child1 = new FibTask(n-1);
			FibTask* child2 = new FibTask(n-2);
			add_successor(child1);
			add_successor(child2);
			SumTask* local_sum =  new SumTask;
			child1->generate_DAG(local_sum);
			child2->generate_DAG(local_sum);
			local_sum->add_successor(sum);
		}
	}
	
	void run() {
		n < 2 ? result = (void*) new ulong(n) : result = (void*) new ulong(0);
	}
};

ulong seq_fib(ulong n) {
	return n < 2 ? n : seq_fib(n-1) + seq_fib(n-2);
}

int main (int argc, char const *argv[]) {
	if (argc != 5) {
		printf("usage: %s X in Y vps\n", argv[0]);
		exit(1);
	}
	
	SumTask sum;
	ulong n = (ulong)atol(argv[1]);
	FibTask fibonacci(n);
	fibonacci.generate_DAG(&sum);
	std::list<Task*> input_nodes;
	input_nodes.push_back(&fibonacci);
	Scheduler::init(atoi(argv[3]),input_nodes);
	Scheduler::terminate();
	printf("It should be %lu\n", seq_fib(n));
	return 0;
}