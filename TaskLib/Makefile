all: serial_object vp task scheduler

serial_object: serial_object.cpp serial_object.h 
	g++ -c serial_object.cpp -o serial_object.o
	
vp: vp.cpp vp.h
	g++ -c vp.cpp -o vp.o
	
task: task.cpp task.h
	g++ -c task.cpp -o task.o

scheduler: scheduler.cpp scheduler.h
	g++ -c scheduler.cpp -o scheduler.o

clean:
	rm *.o