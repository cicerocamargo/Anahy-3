/* objects of this class serve as key 
	for a map<JobId, Job*> in AnahyVM */


#ifndef JOBID_H
#define JOBID_H

#include "definitions.h"

class JobId {

	uint vp_id;
	ulong serial_number;

public:
	JobId();
	JobId(uint _vp_id, ulong _serial_number);
	bool operator<(const JobId& id) const;
	bool operator==(const JobId& id) const;
	void display();

	/* getters and setters */
	uint get_vp_id() const;
	ulong get_serial_number() const;
};

#endif