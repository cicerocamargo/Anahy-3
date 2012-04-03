CFLAGS = -g

all: serial_object vp task scheduler

serial_object: serial_object.cpp serial_object.h 
	g++ $(CFLAGS) -c serial_object.cpp -o serial_object.o
	
vp: vp.cpp vp.h
	g++ $(CFLAGS) -c vp.cpp -o vp.o
	
task: task.cpp task.h
	g++ $(CFLAGS) -c task.cpp -o task.o

scheduler: scheduler.cpp scheduler.h
	g++ $(CFLAGS) -c scheduler.cpp -o scheduler.o

clean:
	rm *.o
