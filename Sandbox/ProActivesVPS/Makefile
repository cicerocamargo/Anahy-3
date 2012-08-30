CPP = g++
CFLAGS = -O3 -g

all: vp job jobid jobattributes anahy

vp: VirtualProcessor.cpp
	$(CPP) $(CFLAGS) VirtualProcessor.cpp -c

job: Job.cpp
	$(CPP) $(CFLAGS) Job.cpp -c

jobid: JobId.cpp
	$(CPP) $(CFLAGS) JobId.cpp -c

anahy: AnahyVM.cpp
	$(CPP) $(CFLAGS) AnahyVM.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) $(CFLAGS) JobAttributes.cpp -c

clean:
	rm -f *.o *.out
