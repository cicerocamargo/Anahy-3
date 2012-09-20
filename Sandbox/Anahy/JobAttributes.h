#ifndef JOBATTRIBUTES_H
#define JOBATTRIBUTES_H

#include "definitions.h"

enum JobAttributes_State { 
	ATHREAD_CREATE_JOINABLE, //default
	ATHREAD_CREATE_DETACHED
};

class JobAttributes {

	int num_joins;
	bool initialized;
	JobAttributes_State detach_state;
	int job_cost;

public:
	JobAttributes();

	bool dec_join_counter();

	//getters and setters
	void set_num_joins(int _num_joins);
	void set_detach_state(JobAttributes_State _detach_state);
	void set_job_cost(int _job_cost);
	void set_initialized(bool _initialized);

	int get_num_joins() const;
	bool get_initialized() const;
	JobAttributes_State get_JobAttributes_State();
	int get_job_cost() const;
};

#endif