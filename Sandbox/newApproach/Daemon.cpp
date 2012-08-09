#include "Daemon.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

/**** PRIVATE METHODS ****/

/* This method is too large and "ugly?"*/

void* Daemon::run_daemon(void* arg) {
	Daemon* d = (Daemon*) arg;
	d->run();
	return NULL;
}

Daemon::Daemon(int _num_vps) : num_vps(_num_vps) {
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	printf("DAEMON: Creating vps\n");
	//create my vps
	for(int i = 0; i < num_vps; i++) {
		vps_running.push_back(new VirtualProcessor(this));
	}
}

bool Daemon::work_stealing_function(VirtualProcessor* vp) {
	printf("DAEMON: I'll steal a job\n");
	
	Job* job = NULL;

 	list<VirtualProcessor*>::iterator it;
 	for(it = vps_running.begin(); it != vps_running.end(); ++it) {

		/* false indicates to make a theft*/
		job = (*it)->get_ready_job(NULL, false);

		if(job) {
			printf("DAEMON: I've found a job for vp %d\n", vp->get_id());
 			/*if the Daemon find a job to vp, 
	 		* but it has already recepted a new 
 			* job from create primitive, the Daemon
 			* need to get the next vp on waiting list
 			*/
	 		
	 		job->set_vp_thief(vp);
 			vp->set_current_job(job);

 			vps_running.push_back(vp);
 			
 			return false;
 		}
 	}
 	return true;
}

void Daemon::run() {
 	//should_stop = false;
 	printf("DAEMON: Running...\n");
 	start_my_vps();

 	bool job_not_found = true;

 	VirtualProcessor* vp;

 	pthread_mutex_lock(&mutex);
 	while (true) {
 		if(vps_waiting.size() == num_vps) {
 			//all my vps are waiting
 			printf("DAEMON: All vps are wainting,\n");
 			pthread_mutex_unlock(&mutex);
 			broadcast_null_job();
 			break;
 		}
 		else {
 			if(vps_waiting.empty()) {
 				printf("DAEMON: Without jobs on waiting list\n");
 				pthread_cond_wait(&cond, &mutex);
 			}
 			else {
 				vp = vps_waiting.front();
 				vps_waiting.pop_front();
 				__sync_sub_and_fetch(&num_vps_waiting, 1);

 				/* here is where the magic happens */
 				job_not_found = work_stealing_function(vp);

 				if(job_not_found) {
 					printf("Any job has been found, putting vp %d into waiting againg\n", vp->get_id());
 					vps_waiting.push_front(vp);
 					pthread_cond_wait(&cond, &mutex);
 				}
 				else {
 					vp->resume();
 					pthread_mutex_unlock(&mutex);
 				}
 			}
 		}
	}
	
	stop_my_vps();
}

Daemon::~Daemon() {
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	vps_waiting.clear();
	vps_running.clear();
}

void Daemon::start_my_vps() {
	/* I still have the lock*/
	printf("DAEMON: Starting the vps\n");
	list<VirtualProcessor*>::iterator it;

	for (it = vps_running.begin(); it != vps_running.end(); ++it) {
		if((*it)->get_id() == 0) {
			printf("DAEMON: Main_vp will be associated\n");
			AnahyVM::set_main_vp(*it);
			
		} else {
			(*it)->start(); // start vps
		}
	}

	printf("DAEMON: All vps has started\n");
}

void Daemon::stop_my_vps() {

	list<VirtualProcessor*>::iterator it;
	/* this allows the main VP to help the execution of
	 * remaining jobs and the Daemon to know that the
	 * main VP is also idle when there's no work
	 */

	printf("DAEMON: Stoping vps\n");
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

void Daemon::broadcast_null_job() {
	for (list<VirtualProcessor*>::iterator i = vps_waiting.begin(); i != vps_waiting.end(); ++i) {
		(*i)->set_current_job(NULL);
		(*i)->resume();
	}
}

void Daemon::waiting_for_a_job(VirtualProcessor* vp) {
	__sync_add_and_fetch(&num_vps_waiting, 1);

	pthread_mutex_lock(&mutex);

	printf("VP %d has added itself on waiting list\n", vp->get_id());

	vps_waiting.push_back(vp);
	vps_running.remove(vp);

	/*the waiting list has modified, then
	we've got to wake up the Daemon and
	block the new vp on the waiting list
	*/
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	vp->block();
}

// called from AnahyVM
void Daemon::start() {
	// create my own thread
	pthread_create(&thread, NULL, run_daemon, this); 
}

void Daemon::stop() {
	// join my own thread
	pthread_join(thread, NULL);
}