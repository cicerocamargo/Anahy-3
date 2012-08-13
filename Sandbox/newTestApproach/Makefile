CPP = g++
CFLAGS = -O3

all: daemon vp job jobid jobgraph jobattributes anahy

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

anahy: AnahyVM.cpp
	$(CPP) $(CFLAGS) AnahyVM.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) $(CFLAGS) JobAttributes.cpp -c

clean:
	rm -f *.o *.out
