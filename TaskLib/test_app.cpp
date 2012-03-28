#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "task.h"
#include "scheduler.h"

class RandNumTask : public Task {
	double* rand_num;
public:
	RandNumTask() {
		rand_num = new double(rand() % 100);
	}
	void run() {
		printf("Sou a tarefa %ld e gerei %f\n", get_id(), *rand_num);
		result = (void*) rand_num;
	}
};

class PowerTask : public Task {
	double power, res;
public:
	PowerTask(double p) : power(p) {}
	void run() {
		double* pred_res = (double*)(*(predecessors.begin()))->get_result();
		res = pow(*pred_res,power);
		printf("Sou a tarefa %ld e elevei %f na %f potencia (igual a %f))\n", get_id(), *pred_res, power, res);
		result = &res;
	}
};

class SumTask : public Task {
	
public:
	SumTask();
	void run() {
		double sum = 0.0;
		for (it = predecessors.begin(); it != predecessors.end(); it++) {
			double* res = (double*) (*it)->get_result();
			sum += (*res);
		}
		printf("Sou a tarefa %ld e a soma eh: %f\n", get_id(), sum);
	}
};

int main(int argc, char** argv) {
	srand(time(NULL));
	
	RandNumTask a;
	PowerTask b(2.0), c(3.0), d(4.0);
	SumTask e;
	a.add_successor(&b);
	a.add_successor(&c);
	a.add_successor(&d);
	b.add_successor(&e);
	c.add_successor(&e);
	c.add_successor(&e);
	std::list<Task*> input_nodes;
	input_nodes.push_back(&a);
	Scheduler::init(2, input_nodes);
	
	return 0;
}