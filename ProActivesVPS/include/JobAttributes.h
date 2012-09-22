#ifndef JOBATTRIBUTES_H
#define JOBATTRIBUTES_H

#include "definitions.h"

class JobAttributes {

	bool initialized;
	int num_joins;
	int jobCost; //1 - default
	
public:

	JobAttributes();
	~JobAttributes();

	bool dec_join_counter();

	//getters and setters
	void set_num_joins(int _num_joins) { num_joins = _num_joins; }
	void set_job_cost(int _job_cost) { jobCost = _job_cost; }
	void set_initialized(bool _initialized) { initialized = _initialized; }

	inline bool get_initialized() const { return initialized; }
	inline int get_job_cost() { return jobCost; }
};

#endif