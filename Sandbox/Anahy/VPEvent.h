#ifndef VPEVENT_H
#define VPEVENT_H

#include "Job.h"
#include "VirtualProcessor.h"

enum VPEventType{ NewJob, EndOfJob, GetJob, DestroyJob };

class VPEvent {

	VPEventType type;
	Job* associated_job;
	VirtualProcessor* sender;

public:
	VPEvent(VPEventType _type, Job* _associated_job, VirtualProcessor* _sender);

	/* getters and setters */
	inline VPEventType get_type() const { return type; }
	inline Job* get_associated_job() const { return associated_job; }
	inline VirtualProcessor* get_sender() const { return sender; }
};

#endif