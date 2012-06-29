#include "Daemon.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

pthread_mutex_t Daemon::mutex;
pthread_cond_t Daemon::cond;

VirtualProcessor* Daemon::main_vp = 0;
list<VirtualProcessor*> Daemon::vps;
list<VirtualProcessor*> Daemon::vps_waiting;

int Daemon::num_vps = 0;
int Daemon::num_vps_waiting = 0;

void Daemon::start_my_vps() {
	list<VirtualProcessor*>::iterator it;

	for (it = vps.begin(); it != vps.end(); ++it) {
		if((*it)->get_id() == 0) {
			set_main_vp(*it);
		} else {
			(*it)->start(); // start daemons
		}
	}
}

void Daemon::stop_my_vps() {
	list<VirtualProcessor*>::iterator it;

	main_vp->run();

	for (it = vps.begin(); it != vps.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
}

//here the interface begins to be described
void Daemon::init(int _num_vps) {
	num_vps = _num_vps;
	VirtualProcessor::init_pthread_key();

	for(int i = 0; i < num_vps; ++i) {
		vps.push_back(new VirtualProcessor(this));
	}

	pthread_mutex_init(&mutex, NULL);
	/* since the main thread has the VM's lock,
	* it can block itself in the next call
	* to wait for the VP 0 to be associated
	* with the main thread*/
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set
								// by daemon 0
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	pthread_mutex_unlock(&mutex);

	start();
}

void Daemon::terminate() {
	stop();

	list<VirtualProcessor*>::iterator it;

	for (it = vps.begin(); it != vps.end(); ++it) {
		delete *it; // destroy vps
	}
	vps.clear();

	//pthread_cond_destroy(&cond);
	//pthread_mutex_destroy(&mutex);
	VirtualProcessor::delete_pthread_key();
}

void Daemon::create(JobHandle* handle, JobAttributes* attr,
	pfunc function, void* args) {

	VirtualProcessor* vp = VirtualProcessor::get_current_vp();
	*handle = vp->create_new_job(function, args, attr);
}

void Daemon::join(JobHandle handle, void** result) {
	VirtualProcessor* vp = VirtualProcessor::get_current_vp();

	void* temp = vp->join_job(handle);

	if(result) {
		*result = temp;
	}
}

void Daemon::set_main_vp(VirtualProcessor* vp) {
	main_vp = vp;
	pthread_mutex_unlock(&mutex);
}

void* Daemon::run_daemon(void* arg) {
	Daemon* d = (Daemon*) arg;
	d->run();
	return NULL;
}

void Daemon::answer_oldest_vp_waiting(Job* job) {
	// job's state has already been set to running
	VirtualProcessor* vp;

	vp = vps_waiting.front();
	vps_waiting.pop_front();
	vp->set_current_job(job);	// send a NULL job to
								// break VP loop
	vp->resume();
}

// void Daemon::run() {
// 	should_stop = false;

// 	start_my_vps();

// 	pthread_mutex_lock(&mutex);
// 	while (true) {
// 		if (event_queue.empty()) {
// 			if (vps_waiting.size() == num_vps) {
// 				// all vps are waiting
// 				pthread_mutex_unlock(&mutex);

// 				Job* j = AnahyVM::blocking_get_job(this);

// 				if (should_stop) {
// 					while (!vps_waiting.empty()) {
// 						answer_oldest_vp_waiting(NULL);
// 					}
// 					break;
// 				}

// 				if (j) {
// 					answer_oldest_vp_waiting(j);
// 				}

// 				pthread_mutex_lock(&mutex);
// 			}
// 			else {
// 				pthread_cond_wait(&event_cond, &mutex);
// 			}
// 		}
// 		else {
// 			VPEvent event = event_queue.front();
// 			event_queue.pop();

// 			pthread_mutex_unlock(&mutex);
// 			handle_event(event);

// 			pthread_mutex_lock(&mutex);
// 		}
// 	}

// 	stop_my_vps();
// }