#include "AnahyJob.h"
#include "AnahyJobAttributes.h"

int AnahyJob::counter = 0;

AnahyJob::AnahyJob(ParFunc function, void* args, AnahyJobAttributes* attr, bool smart) {
	assert(function);
	_id = counter++;
	_function = function;
	_args = args;
	_smart = smart;
	_fork_counter = 1;
	_join_counter = 1;
	_attr = attr;
	_state = AnahyJobStateReady;
}

AnahyJob* AnahyJob::new_smart_job(ParFunc function, void* args, AnahyJobAttributes* attr) { 
	return new AnahyJob(function, args, attr, true);
}


AnahyJob::~AnahyJob() {
	if (_attr && _smart && _attr->smart()) {
		delete _attr;
	}
}

void AnahyJob::set_join_counter(unsigned short join_counter) {
	assert(join_counter);
	_join_counter = join_counter;
}

void AnahyJob::set_fork_counter(unsigned short fork_counter) {
	assert(fork_counter);
	_fork_counter = fork_counter;
}


void AnahyJob::run() {
    _result = (_function)(_args);
    compare_and_swap_state(AnahyJobStateRunning, AnahyJobStateFinished);
}

