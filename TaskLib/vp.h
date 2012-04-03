#ifndef VP_H
#define VP_H

#include "task.h"
#include "serial_object.h"

class Scheduler;


class VirtualProcessor {
	static bool program_running;
	static int vps_waiting;
	static unsigned long int instance_counter;
	
	unsigned long int id;
	unsigned long int affinity_mask;

public:
	static void set_program_running(bool state);
	static int get_vps_waiting();
	VirtualProcessor();
	VirtualProcessor(unsigned long int aff_mask);
	void run();
};

#endif