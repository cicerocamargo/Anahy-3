#include "VPEvent.h"

VPEvent::VPEvent(VPEventType _type, Job* _associated_job, VirtualProcessor* _sender)
: type(_type), associated_job(_associated_job), sender(_sender) {}