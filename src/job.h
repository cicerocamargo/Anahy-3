#ifndef JOB_H
#define JOB_H

class Job {
	static unsigned long counter;
	unsigned long id;
	void* (*func)(void*);
	void* args;
	
public:
	Job(void* (*f)(void*), void* func_args);
	//virtual ~Job ();
	unsigned long get_id();
	void run();
};

#endif JOB_H