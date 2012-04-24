#include <cstdio>
#include "Job.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "athread.h"

void* hello(void* args) {
	puts("Hello World!");
	return NULL;    
}

int main (int argc, char const *argv[]) {
	Daemon* d = new Daemon();
	VirtualProcessor* vp = new VirtualProcessor(d);
	Job* j = new Job(vp->get_new_JobId(), NULL, vp, 0, hello, NULL);
	j->display();
	
	j->run();
	printf("retval: %p\n", j->get_retval());
	void* new_retval;
	j->set_retval(new_retval);
	printf("retval: %p\n", j->get_retval());
	Job* j2 = new Job(vp->get_new_JobId(), j, vp, 0, hello, NULL);
	j->display();
	return 0;
}