#include "WorkerEvent.h"
#include "Worker.h"
#include "Work.h"

WorkerEvent::WorkerEvent(WorkerEventType type, Worker* sender, Work* work){
	this->type = type;
	this->sender = sender;
	this->work = work;
}
