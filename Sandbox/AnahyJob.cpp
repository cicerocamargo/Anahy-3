#include "AnahyJob.h"

inline AnahyJob::AnahyJob(ParFunc function, void* args, AnahyJobAttributes* attr, bool smart) {
	assert(function);
	_function = function;
	_args = args;
	_smart = smart;
	_fork_counter = 1;
	_join_counter = 1;
	_attr = attr;
	_state = AnahyJobStateReady;
}

inline AnahyJob* AnahyJob::new_smart_job(ParFunc function, void* args, AnahyJobAttributes* attr) { 
	return new AnahyJob(function, args, attr, true);
}


inline AnahyJob::~AnahyJob() {
	if (_attr && this->smart() && _attr->smart()) {
		delete _attr;
	}
}

inline void AnahyJob::set_join_counter(unsigned short join_counter) {
	assert(join_counter);
	_join_counter = join_counter;
}

inline void AnahyJob::set_fork_counter(unsigned short fork_counter) {
	assert(fork_counter);
	_fork_counter = fork_counter;
}

// the return value indicates operation's success (true) or failure
inline bool AnahyJob::compare_and_swap_state(AnahyJobState target_value, AnahyJobState new_value) {
	return __sync_bool_compare_and_swap (&_state, target_value, new_value);
}

void AnahyJob::run() {
    _result = (_function)(_args);
    compare_and_swap_state(AnahyJobStateRunning, AnahyJobStateFinished);
}