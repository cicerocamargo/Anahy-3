#include "vp.h"
#include "scheduler.h"
#include "check.h"
#include <stdio.h>
#include <stdlib.h>

bool VirtualProcessor::program_running = false;
int VirtualProcessor::vps_waiting = 0;
unsigned long int VirtualProcessor::instance_counter = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

VirtualProcessor::VirtualProcessor() {
	id = instance_counter++;
}

VirtualProcessor::VirtualProcessor(unsigned long int aff_mask)
: affinity_mask(aff_mask) {
	VirtualProcessor();
}

void VirtualProcessor::set_program_running(bool state) {
	program_running = state;
}

int VirtualProcessor::get_vps_waiting() {
	return vps_waiting;
}

void VirtualProcessor::run() {
	Task* task = NULL;
	int ret_code;
	
	printf("VP %ld running on thread %ld.\n", this->id, (long)pthread_self());
	for ( ; ; ) {
		ret_code = pthread_mutex_lock(&mutex);
		checkResults("pthread_mutex_lock", ret_code);
		if (!program_running) {
			ret_code = pthread_mutex_unlock(&mutex);
			checkResults("pthread_mutex_unlock", ret_code);
			break;
		}
		else { // program is running
			task = Scheduler::task_request();
			
			if (task) {
				ret_code = pthread_mutex_unlock(&mutex);
				checkResults("pthread_mutex_unlock", ret_code);
				task->run();
				//printf("I'm VP %ld and finished task %lu\n", this->id, task->get_id());
				// sends the mutex and conditional var with the last task
				Scheduler::graph_update_request(task, &mutex, &cond);
			}
			else {
				vps_waiting++;
				if (vps_waiting == instance_counter) {
					//printf("I'm VP %ld and I say the game is over! muá-ha-ha-ha!!!\n", this->id);
					program_running = false;
					pthread_cond_broadcast(&cond);
					checkResults("pthread_cond_broadcast", ret_code);
					ret_code = pthread_mutex_unlock(&mutex);
					checkResults("pthread_mutex_unlock", ret_code);
				}
				else {
					//printf("VP %ld blocked!\n", this->id);
					ret_code = pthread_cond_wait(&cond, &mutex);
					checkResults("pthread_cond_wait", ret_code);
					vps_waiting--;
					ret_code = pthread_mutex_unlock(&mutex);
					checkResults("pthread_mutex_unlock", ret_code);
				}
			}
		}
	}
}
