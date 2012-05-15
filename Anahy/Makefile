CPP = g++

vp: VirtualProcessor.cpp
	$(CPP) VirtualProcessor.cpp -c

job: Job.cpp
	$(CPP) Job.cpp -c

jobid: JobId.cpp
	$(CPP) JobId.cpp -c

daemon: Daemon.cpp
	$(CPP) Daemon.cpp -c

anahyvm: AnahyVM.cpp
	$(CPP) AnahyVM.cpp -c

vpevent: VPEvent.cpp
	$(CPP) VPEvent.cpp -c

athread: athread.cpp
	$(CPP) athread.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) JobAttributes.cpp -c

env: daemon anahyvm vpevent vp job jobid jobattributes
	
job_test: Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o VPEvent.o athread.o
	$(CPP) Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o VPEvent.o athread.o job_test.cpp -o job_test

athread_test: Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o VPEvent.o athread.o
	$(CPP) Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o VPEvent.o athread.o JobAttributes.o athread_test.cpp -o athread_test
	
clean:
	rm -f *.o *.out
