#include "JobId.h"
#include <cstdio>

JobId::JobId() {
	vp_id = 0;
	serial_number = 0;
}

JobId::JobId(uint _vp_id, ulong _serial_number)
: vp_id(_vp_id), serial_number(_serial_number) {}

bool JobId::operator<(const JobId& id) const {
	if (serial_number == id.serial_number) {
		return (vp_id < id.vp_id);
	}
	else {
		return (serial_number < id.serial_number);
	}
}

bool JobId::operator==(const JobId& id) const {
 	return (vp_id == id.vp_id && serial_number == id.serial_number);
}

void JobId::display() {
	printf("vp%djob%lu", vp_id, serial_number);
}

/* getters and setters */
uint JobId::get_vp_id() const {
	return vp_id;
}

ulong JobId::get_serial_number() const {
	return serial_number;
}
