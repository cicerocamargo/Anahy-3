#include <cstdio>
#include "Job.h"
#include "Daemon.h"
#include "VirtualProcessor.h"

void* hello(void* args) {
	puts("Hello World!");
        return NULL;    
}

int main (int argc, char const *argv[]) {
        Daemon* d = new Daemon();
	VirtualProcessor* vp = new VirtualProcessor(d);
	Job* j = new Job(vp->get_new_JobId(), NULL, vp, 0, hello, NULL);
        j->run();
	printf("%p\n", j->get_retval());
	return 0;
}