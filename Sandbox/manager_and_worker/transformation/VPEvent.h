class VirtualProcessor;
class Job;

enum VPEventType { GetJob, NewJob };

class VPEvent {
	VPEventType type;
	VirtualProcessor* sender;
	Job* job;
	
public:
	inline VPEvent(VPEventType _type, VirtualProcessor* _sender, Job* _job) :
		type(_type), sender(_sender), job(_job) {}

	inline VPEventType get_type() { return type; }
	inline VirtualProcessor* get_sender() { return sender; }
	inline Job* get_job() { return job; }
};