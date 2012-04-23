#include <cstdio>
#include "Job.h"

void* hello(void* args) {
	puts("Hello World!");
}

int main (int argc, char const *argv[]) {
	VirtualProcessor* vp = new VirtualProcessor(new Deamon());
	Job* j = new Job(vp->get_new_JobId(), NULL, vp, 0, hello, NULL);
	j->run();
	return 0;
}