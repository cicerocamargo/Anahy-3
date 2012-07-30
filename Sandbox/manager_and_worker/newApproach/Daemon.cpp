#include "Daemon.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

/**** STATIC MEMBERS' ITIALIZATIONS ****/

pthread_mutex_t Daemon::mutex;
pthread_cond_t Daemon::cond;

VirtualProcessor* Daemon::main_vp = 0;
list<VirtualProcessor*> Daemon::vps_waiting;
list<VirtualProcessor*> Daemon::vps_running;

int Daemon::num_vps = 0;
int Daemon::num_vps_waiting = 0;

/**** PRIVATE METHODS ****/

void Daemon::answer_oldest_vp_waiting(Job* job) {
	// job's state has already been set to running
	VirtualProcessor* vp;

	vp = vps_waiting.front();
	vps_waiting.pop_front();
	vp->set_current_job(job);	// send a NULL job to
								// break VP loop
	__sync_sub_and_fetch(&num_vps_waiting, 1);
	vp->resume();
}

void Daemon::run() {
 	//should_stop = false;
 	start_my_vps();

 	VirtualProcessor* vp;
 	Job* job;
 	pthread_mutex_lock(&mutex);
 	while (true) {
 		if(vps_waiting.size() == num_vps) {
 			//all my vps are waiting
 			pthread_mutex_unlock(&mutex);
 			break;
 		}
 		else {
 			if(vps_waiting.empty()) {
 				pthread_cond_wait(&cond, &mutex);
 			}
 			else {
 				vp = vps_waiting.front();
 				vps_waiting.pop_front();
 				pthread_mutex_unlock(&mutex);
 				bool job_not_found = true;
 				 
 				list<VirtualProcessor*>::iterator it;
 				for(it = vps_running.begin(); it != vps_running.end(); it++) {

 					/* we cannot find a job on vp from vps_waiting list*/
 					if((*it)->get_id() == vp->get_id()) {
 						continue;
 					}

 					job = (*it)->get_ready_job(NULL);

 					if(job) {
 						/* if the Daemon find a job to vp, 
 						but it has already recepted a new 
 						job from create primitive, the Daemon
 						need to get the next vp on waiting list
 						*/
 						pthread_mutex_lock(&mutex);

 						if(!vp->get_status()) {
 							vp = vps_waiting.front();
 							vps_waiting.pop_front();

 							pthread_mutex_unlock(&mutex);
 						}
 						job->set_vp_thief(vp);
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

void Daemon::start_my_vps() {
	list<VirtualProcessor*>::iterator it;

	//remove vps list
	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() == 0) {
			set_main_vp(*it);
		} else {
			(*it)->start(); // start vps
		}
	}
	pthread_mutex_lock(&mutex);

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

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
		}
	}
	if(!vps_waiting.empty()) {
		for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
			if((*it)->get_id() > 0) {
				(*it)->stop();
			}
		}
	}
}

/**** PUBLIC METHODS ****/

//here the interface begins to be described
void Daemon::init(int _num_vps) {
	num_vps = _num_vps;
	VirtualProcessor::init_pthread_key();

	for(int i = 0; i < num_vps; ++i) {
		vps_running.push_back(new VirtualProcessor());
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

		//stop_my_vps();

		for (it = vps_running.begin(); it != vps_running.end(); ++it) {
			delete *it; // destroy vps
		}
		if(!vps_waiting.empty()) {
			for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
				delete *it; // destroy vps
			}
		}

		vps_running.clear();
		vps_waiting.clear();

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
/*
void Daemon::broadcast_null_job() {
	for (list<VirtualProcessor*>::iterator i = vps_waiting.begin(); i != vps_waiting.end(); ++i) {
		(*i)->set_current_job(NULL);
		(*i)->resume();
	}
}
*/

void Daemon::waiting_for_a_job(VirtualProcessor* vp) {

	__sync_add_and_fetch(&num_vps_waiting, 1);

	pthread_mutex_lock(&mutex);

	vps_waiting.push_back(vp);
	vps_running.remove(vp);

	/*the waiting list has modified, then
	we've got to wake up the Daemon and
	block the new vp on the waiting list
	*/
	vp->block();
	pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);	
}

void Daemon::remove_vp_from_waiting_list(VirtualProcessor* vp) {
	__sync_sub_and_fetch(&num_vps_waiting, 1);

	pthread_mutex_lock(&mutex);

	vps_running.push_back(vp);
	vps_waiting.remove(vp);

	vp->resume();

	pthread_mutex_unlock(&mutex);
}

void Daemon::set_main_vp(VirtualProcessor* vp) {
	main_vp = vp;
	pthread_mutex_unlock(&mutex);
}

void Daemon::start() {
	run();
	return;
}