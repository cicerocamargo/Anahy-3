#ifndef JOBATTRIBUTES_H
#define JOBATTRIBUTES_H

#include "definitions.h"

enum JobCost {
	MINIMUM_COST,
	NORMAL_COST,
	MAXIMUM_COST,
};

class JobAttributes {

	bool initialized;
	int num_joins;
	JobCost job_cost; //1 - default
	
public:

	JobAttributes();
	~JobAttributes();

	bool dec_join_counter();

	//getters and setters
	void set_num_joins(int _num_joins) { num_joins = _num_joins; }
	void set_job_cost(JobCost _job_cost) { job_cost = _job_cost; }
	void set_initialized(bool _initialized) { initialized = _initialized; }

	inline bool get_initialized() const { return initialized; }
	inline int get_job_cost() { return job_cost; }
};

#endif