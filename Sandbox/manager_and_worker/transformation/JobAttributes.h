#ifndef JOBATTRIBUTES_H
#define JOBATTRIBUTES_H

#include "definitions.h"

typedef uint JobAttributes;

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
	JobAttributes(uint _num_joins, bool _initialized, 
		JobAttributes_State _attr_state, int _job_cost);
	~JobAttributes();

	//getters and setters
	uint get_num_joins() const;
	bool get_initialized() const;
	JobAttributes_State get_JobAttributes_State();
	int get_job_cost() const;
};

#endif