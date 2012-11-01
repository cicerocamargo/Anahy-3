#ifndef ANAHYJOBATTRIBUTES_H
#define ANAHYJOBATTRIBUTES_H

#include "definitions.h"

enum JobCost {
	LIGHT_COST,
	NORMAL_COST,
	HEAVY_COST,
};

class AnahyJobAttributes {

	bool initialized;
	bool _smart;
	JobCost job_cost; //1 - default
	
public:

	AnahyJobAttributes(bool smart=false);
	AnahyJobAttributes* new_smart_attributes() { return new AnahyJobAttributes(true); }

	void set_job_cost(JobCost _job_cost) { job_cost = _job_cost; }
	void set_initialized(bool _initialized) { initialized = _initialized; }

	bool get_initialized() const { return initialized; }
	int get_job_cost() { return job_cost; }
	bool smart() { return _smart; }

};

#endif