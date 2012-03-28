#ifndef VP_H
#define VP_H

#include "task.h"
#include "serial_object.h"

class Scheduler;

enum VPState { VPsleeping, VPrunning };

class VirtualProcessor : public SerialObject {
	static bool program_running;
	VPState state;
	unsigned long int affinity_mask;

public:
	static void set_program_running(bool state);
	VirtualProcessor(unsigned long int aff_mask);
	VirtualProcessor();
	~VirtualProcessor();
	void set_state(VPState state);
	VPState get_state();
	void run();
};

#endif