#include "Daemon.h"
#include "Job.h"
#include "JobGraph.h"
#include "VirtualProcessor.h"
#include <cstdio>
#include <cstdlib>

JobGraph Daemon::graph;

pthread_mutex_t AnahyVM::mutex;
pthread_cond_t AnahyVM::cond;

VirtualProcessor* Daemon::main_vp = 0;
list<VirtualProcessor*> Daemon::vps;
list<VPEvent> Daemon::vps_waiting;

int Daemon::num_vps = 0
int Daemon::vps_waiting = 0;

void Daemon::start() {
	list<Daemon*>::iterator it;

	for (it = vps.begin(); it != vps.end(); ++it) {
		if((*it)->get_id() == 0) {
			set_main_vp(*it);
		} else {
			(*it)->start(); // start daemons
		}
	}
	pthread_mutex_lock(&mutex);	// wait for VP 0 to be set
								// by daemon 0
	VirtualProcessor::associate_vp_with_current_thread((void*) main_vp);
	pthread_mutex_unlock(&mutex);
}

void Daemon::stop() {
	list<VirtualProcessor*>::iterator it;

	main_vp->run();


	for (it = daemons.begin(); it != daemons.end(); ++it) {
		if((*it)->get_id() > 0) {
			(*it)->stop();
	}
}

void Daemon::init(int _num_vps) {
	num_vps = _num_vps;
	VirtualProcessor::init_pthread_key();

	for(int i = 0; i < num_vps; ++i) {
		vps.push_back(new VirtualProcessor(this));
	}

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);

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

	VPEvent event = vps_waiting.front();
	vps_waiting.pop_front();
	vp = event.get_sender();
	vp->set_current_job(job);	// send a NULL job to
								// break VP loop
	vp->resume();
}

void Daemon::run() {
	should_stop = false;

	start_my_vps();

	pthread_mutex_lock(&mutex);
	while (true) {
		if (event_queue.empty()) {
			if (vps_waiting.size() == num_vps) {
				// all vps are waiting
				pthread_mutex_unlock(&mutex);

				Job* j = AnahyVM::blocking_get_job(this);

				if (should_stop) {
					while (!vps_waiting.empty()) {
						answer_oldest_vp_waiting(NULL);
					}
					break;
				}

				if (j) {
					answer_oldest_vp_waiting(j);
				}

				pthread_mutex_lock(&mutex);
			}
			else {
				pthread_cond_wait(&event_cond, &mutex);
			}
		}
		else {
			VPEvent event = event_queue.front();
			event_queue.pop();

			pthread_mutex_unlock(&mutex);
			handle_event(event);

			pthread_mutex_lock(&mutex);
		}
	}

	stop_my_vps();
}


void Daemon::handle_get_job(VPEvent event) {
	VirtualProcessor* vp;
	Job* job;

	job = AnahyVM::get_job(event);
	if (!job) {
		vps_waiting.push_back(event);	
	}
	else {
		// here job's state is already running
		VirtualProcessor* vp = event.get_sender();
		vp->set_current_job(job);
		vp->resume();
	}
}

void Daemon::handle_new_job(VPEvent event) {
	Job* job = event.get_job();
	if (!vps_waiting.empty() && job->compare_and_swap_state(ready, running)) {
		if (event.get_fwd() == false) {
			AnahyVM::post_job(event, true);
		}
		answer_oldest_vp_waiting(job);
	}
	else {
		if (event.get_fwd() == false) {
			AnahyVM::post_job(event, false);
		}
	}
}

void Daemon::handle_end_of_job(VPEvent event) {
	VirtualProcessor* vp;
	Job* job = event.get_job();

	if (event.get_fwd() == false) {
		AnahyVM::forward_end_of_job(event);
	}

	list<VPEvent>::iterator it;
	for (it = vps_waiting.begin(); it != vps_waiting.end(); ++it) {
		if ((*it).get_job() == job) {
			// found a VP waiting for the job that just ended !!
			vp = (*it).get_sender();
			it = vps_waiting.erase(it);
			vp->resume();
		}
	}	
}

void Daemon::handle_event(VPEvent event) {

	switch (event.get_type()) {
		case GetJob:
			handle_get_job(event);
			break;
		case NewJob:
			handle_new_job(event);
			break;
		case EndOfJob:
			handle_end_of_job(event);
			break;
		case DestroyJob:
			AnahyVM::erase_job(event.get_job());
			break;
		default:
			fprintf(stderr, "Unknown event on Daemon %d\n", id);
			exit(1);
	}
}

Daemon::Daemon(int n_vps) : num_vps(n_vps) {
	id = instances++;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&event_cond, NULL);

	// create vps
	for (int i = 0; i < num_vps; ++i) {
		vps.push_back(new VirtualProcessor(this));
	}
}

Daemon::~Daemon() {
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&event_cond);

	for (int i = 0; i < num_vps; ++i) {
		delete vps[i];
	}
	vps.clear();

	instances--;
}

void Daemon::push_event(VPEvent event) {
	pthread_mutex_lock(&mutex);
	event_queue.push(event);
	pthread_cond_signal(&event_cond); // wake daemon
	pthread_mutex_unlock(&mutex);
}

// called from AnahyVM
void Daemon::start() {
	// create my own thread
	pthread_create(&thread, NULL, run_daemon, this); 
}

// called from AnahyVM
void Daemon::stop() {
	// join my own thread
	pthread_join(thread, NULL);
}