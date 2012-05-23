#include "JobAttributes.h"

JobAttributes JobAttributes::execution_cost;

JobAttributes::JobAttributes(int _num_joins, bool _initialized, 
		JobAttributes_State _attr_state, int _job_cost) : 
	num_joins(_num_joins),
	initialized(_initialized),
	attr_state(_attr_state),
	job_cost(_job_cost)
{

}

int JobAttributes::get_num_joins() const {
	return num_joins;
}

bool JobAttributes::get_initialized() const {
	return initialized;
}

JobAttributes_State JobAttributes::get_JobAttributes_State() {
	return attr_state;
}

int JobAttributes::get_job_cost() const {
	return job_cost;
}