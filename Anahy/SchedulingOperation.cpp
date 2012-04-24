#include "SchedulingOperation.h"


SchedulingOperation::SchedulingOperation(SchedOpType _type, 
	Job* _associated_job, VirtualProcessor* _applicant)
: type(_type), associated_job(_associated_job), applicant(_applicant) {}

/* getters and setters */
SchedOpType SchedulingOperation::get_type() const {
	return type;
}

Job* SchedulingOperation::get_associated_job() const {
	return associated_job;
}

VirtualProcessor* SchedulingOperation::get_applicant() const {
	return applicant;
}