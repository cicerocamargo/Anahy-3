#include "include/JobAttributes.h"
#include "include/Job.h"

JobAttributes::JobAttributes() {
	set_initialized(false);
	set_num_joins(1);
	set_job_cost(1);
}

JobAttributes::~JobAttributes() {
}

bool JobAttributes::dec_join_counter() {
	if(__sync_bool_compare_and_swap(&num_joins, num_joins, num_joins - 1))
	return __sync_bool_compare_and_swap(&num_joins, 0, -1);
}