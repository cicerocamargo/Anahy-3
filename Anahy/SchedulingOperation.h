#ifndef SCHEDULINGOPERATION_H
#define SCHEDULINGOPERATION_H

class Job;
class VirtualProcessor;

enum SchedOpType { NewJob, EndJob, GetJob };

class SchedulingOperation {

	SchedOpType type;
	Job* associated_job;
	VirtualProcessor* applicant;

public:
	SchedulingOperation(SchedOpType _type, Job* _associated_job, VirtualProcessor* _applicant);
	virtual ~SchedulingOperation();

	/* getters and setters */
	SchedOpType get_type();
	Job* get_associated_job();
	VirtualProcessor* get_applicant();
};

#endif