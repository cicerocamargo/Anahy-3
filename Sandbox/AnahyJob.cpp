#include "AnahyJob.h"

inline AnahyJob(ParFunc function, void* args, AnahyJobAttributes* attr, bool smart=false) {
	assert(function);
	_function = function;
	_args = args;
	_smart = smart;
	_fork_counter = 1;
	_join_counter = 1;
	_attr = attr;
}

inline ~AnahyJob() {
	if (_attr && this->smart() && _attr->smart()) {
		delete _attr;
	}
}

inline AnahyJob* AnahyJob::new_smart_job(ParFunc function, void* args, AnahyJobAttributes* attr) { 
	return new AnahyJob(function, args, attr, true);
}

inline void AnahyJob::set_join_counter(unsigned short join_counter) {
	assert(join_counter);
	_join_counter = join_counter;
}

inline void AnahyJob::set_fork_counter(unsigned short fork_counter) {
	assert(fork_counter);
	_fork_counter = fork_counter;
}