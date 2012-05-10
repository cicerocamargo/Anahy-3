CPP = g++

vp: VirtualProcessor.cpp
	$(CPP) VirtualProcessor.cpp -c

job: Job.cpp
	$(CPP) Job.cpp -c

jobid: JobId.cpp
	$(CPP) JobId.cpp -c

daemon: anahyvm schedop Daemon.cpp
	$(CPP) Daemon.cpp -c

anahyvm: AnahyVM.cpp
	$(CPP) AnahyVM.cpp -c

schedop: SchedulingOperation.cpp
	$(CPP) SchedulingOperation.cpp -c

athread: athread.cpp
	$(CPP) athread.cpp -c

jobattributes: JobAttributes.cpp
	$(CPP) JobAttributes.cpp -c

env: daemon anahyvm schedop vp job jobid jobattributes
	
job_test: Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o SchedulingOperation.o athread.o
	$(CPP) Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o SchedulingOperation.o athread.o job_test.cpp -o job_test

athread_test: Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o SchedulingOperation.o athread.o
	$(CPP) Job.o JobId.o Daemon.o VirtualProcessor.o AnahyVM.o SchedulingOperation.o athread.o JobAttributes.o athread_test.cpp -o athread_test
	
clean:
	rm *.o
