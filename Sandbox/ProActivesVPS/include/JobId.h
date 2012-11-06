#ifndef JOBID_H
#define JOBID_H

#include "definitions.h"

class JobId {

	int vp_id;
	long serial_number;

public:
	JobId();
	JobId(int _vp_id, long _serial_number);
	void display();

	/* getters and setters */
	int get_vp_id() const;
	long get_serial_number() const;
};

#endif