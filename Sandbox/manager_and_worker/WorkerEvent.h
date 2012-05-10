class Worker;
class Work;

enum WorkerEventType { GetWork, PostWork };

class WorkerEvent {
	WorkerEventType type;
	Worker* sender;
	Work* work;
public:
	WorkerEvent(WorkerEventType type, Worker* sender, Work* work);
	inline WorkerEventType get_type() { return type; }
	inline Worker* get_sender() { return sender; }
	inline Work* get_work() { return work; }
};