#include "Daemon.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

pthread_mutex_t Daemon::mutex;
pthread_cond_t Daemon::cond;

VirtualProcessor* Daemon::main_vp = 0;
Daemon::num_vps_waiting = 0;
list<VirtualProcessor*> Daemon::vps;
list<VirtualProcessor*> Daemon::vps_waiting;
list<VirtualProcessor*> Daemon::vps_running;

int Daemon::num_vps = 0;
int Daemon::num_vps_waiting = 0;

void Daemon::start() {
	run();
	return;
}

void Daemon::start_my_vps() {
	list<VirtualProcessor*>::iterator it;
	vps_running = vps;
	//remove vps list
	for (it = vps.begin(); it != vps.end(); ++it) {
		if((*it)->get_id() == 0) {
			set_main_vp(*it);
		} else {
			(*it)->start(); // start vps
		}
	}
	pthread_mutex_lock(&mutex);
	//is this really necessary, now?
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	pthread_mutex_unlock(&mutex);
}

void Daemon::stop_my_vps() {
	list<VirtualProcessor*>::iterator it;
	/* this allows the main VP to help the execution of
	 * remaining jobs and the Daemon to know that the
	 * main VP is also idle when there's no work
	 */
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
		vps.push_back(new VirtualProcessor());
	}

	pthread_mutex_init(&mutex, NULL);
	/* since the main thread has the VM's lock,
	* it can block itself in the next call
	* to wait for the VP 0 to be associated
	* with the main thread*/
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set

	run();
}

void Daemon::terminate() {
	list<VirtualProcessor*>::iterator it;
	if(num_vps_waiting == num_vps) {

		stop_my_vps();

		for (it = vps.begin(); it != vps.end(); ++it) {
			delete *it; // destroy vps
		}
		vps.clear();

		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
		VirtualProcessor::delete_pthread_key();
	}
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

void Daemon::answer_oldest_vp_waiting(Job* job) {
	// job's state has already been set to running
	VirtualProcessor* vp;

	vp = vps_waiting_for_a_job.front();
	vps_waiting_for_a_job.pop_front();
	vp->set_current_job(job);	// send a NULL job to
								// break VP loop
	num_vps_waiting--;
	vp->resume();
}

void Daemon::waiting_for_a_job(VirtualProcessor* vp) {
	pthread_mutex_lock(&mutex);

	num_vps_waiting++;
	vps_waiting.push_back(vp);

	pthread_mutex_unlock(&mutex);
}

void Daemon::run() {
 	//should_stop = false;
 	start_my_vps();

 	VirtualProcessor* vp;
 	Job* job;
 	pthread_mutex_lock(&mutex);
 	while (true) {
 		if(vps_waiting_for_a_job.size() == num_vps) {
 			//all my vps are waiting
 			pthread_mutex_unlock(&mutex);
 			break;
 		}
 		else {
 			if(vps_waiting.size() == 0) {
 				pthread_cond_wait(&cond, &mutex);
 			}
 			else {
 				vp = vps_waiting.pop_front();
 				pthread_mutex_unlock(&mutex);
 				bool job_not_found = true;
 				list<VirtualProcessor*>::iterator it;
 				for(it = vps_running.begin(); it != vps_running.end(); it++) {

 					job = (*it)->get_ready_job(NULL);
 					if(job) {
 						vp->set_current_job(job);
 						vps_running.push_back(vp);
 						vp->resume();
 						job_not_found = false;
 						break;
 					}
 				}
 				pthread_mutex_lock(&mutex);
 				if(job_not_found) {
 					vps_waiting.push_front(vp);
 					pthread_cond_wait(&cond, &mutex);
 				}
 			}
 		}
	}
	stop_my_vps();
}