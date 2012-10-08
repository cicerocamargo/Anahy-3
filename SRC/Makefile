CPP = g++
CFLAGS = -O3 -g -DDEBUG

all: vp job anahy attr

vp: VirtualProcessor.cpp
	$(CPP) $(CFLAGS) VirtualProcessor.cpp -c

job: AnahyJob.cpp
	$(CPP) $(CFLAGS) AnahyJob.cpp -c

anahy: AnahyVM.cpp
	$(CPP) $(CFLAGS) AnahyVM.cpp -c

attr: AnahyJobAttributes.cpp
	$(CPP) $(CFLAGS) AnahyJobAttributes.cpp -c	

clean:
	rm -f *.o *.out
