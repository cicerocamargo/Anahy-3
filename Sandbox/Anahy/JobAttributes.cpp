#include "JobAttributes.h"

JobAttributes::JobAttributes() {
	set_initialized(true);
	set_num_joins(1);
	set_detach_state(ATHREAD_CREATE_JOINABLE);
	set_job_cost(0);
}

bool JobAttributes::dec_join_counter() {
	if(__sync_bool_compare_and_swap(&num_joins, num_joins, num_joins - 1))
	return __sync_bool_compare_and_swap(&num_joins, 0, -1);
}

void JobAttributes::set_num_joins(int _num_joins) {
	num_joins = _num_joins;
}

void JobAttributes::set_detach_state(JobAttributes_State _detach_state) {
	detach_state = _detach_state;
}

void JobAttributes::set_job_cost(int _job_cost) {
	job_cost = _job_cost;
}

void JobAttributes::set_initialized(bool _initialized) {
	initialized = _initialized;
}

int JobAttributes::get_num_joins() const {
	return num_joins;
}

bool JobAttributes::get_initialized() const {
	return initialized;
}

JobAttributes_State JobAttributes::get_JobAttributes_State() {
	return detach_state;
}

int JobAttributes::get_job_cost() const {
	return job_cost;
}