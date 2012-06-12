CPP = g++
CFLAGS = -O3

all: daemon anahyvm vpevent vp job jobid jobgraph jobattributes

vp: VirtualProcessor.cpp
	$(CPP) $(CFLAGS) VirtualProcessor.cpp -c

job: Job.cpp
	$(CPP) $(CFLAGS) Job.cpp -c

jobid: JobId.cpp
	$(CPP) $(CFLAGS) JobId.cpp -c

jobgraph: JobGraph.cpp
	$(CPP) $(CFLAGS) JobGraph.cpp -c

daemon: Daemon.cpp
	$(CPP) $(CFLAGS) Daemon.cpp -c

anahyvm: AnahyVM.cpp
	$(CPP) $(CFLAGS) AnahyVM.cpp -c

vpevent: VPEvent.cpp
	$(CPP) $(CFLAGS) VPEvent.cpp -c

athread: athread.cpp
	$(CPP) $(CFLAGS) athread.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) $(CFLAGS) JobAttributes.cpp -c

clean:
	rm -f *.o *.out
