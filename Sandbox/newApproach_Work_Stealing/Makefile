CPP = g++
CFLAGS = -O3

all: vp job jobid jobgraph agent anahy jobattributes

vp: VirtualProcessor.cpp
	$(CPP) $(CFLAGS) VirtualProcessor.cpp -c

job: Job.cpp
	$(CPP) $(CFLAGS) Job.cpp -c

jobid: JobId.cpp
	$(CPP) $(CFLAGS) JobId.cpp -c

jobgraph: JobGraph.cpp
	$(CPP) $(CFLAGS) JobGraph.cpp -c

agent: Agent.cpp
	$(CPP) $(CFLAGS) Agent.cpp -c

anahy: AnahyVM.cpp
	$(CPP) $(CFLAGS) AnahyVM.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) $(CFLAGS) JobAttributes.cpp -c

clean:
	rm -f *.o *.out
