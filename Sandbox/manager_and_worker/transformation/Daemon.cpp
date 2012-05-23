#include "Daemon.h"
#include "AnahyVM.h"
#include "Job.h"
#include "VirtualProcessor.h"
//#include "VPEvent.h"
#include <cstdio>
#include <cstdlib>

int Daemon::instances = 0;

void* Daemon::run_daemon(void* arg) {
	Daemon* d = (Daemon*) arg;
	d->run();
	return NULL;
}

void Daemon::answer_oldest_vp_waiting(Job* job) {
	VPEvent* event;
	VirtualProcessor* vp;

	event = vps_waiting.front();
	vps_waiting.pop();
	vp = event->get_sender();
	vp->set_current_job(job);	// send a NULL job to
								// break VP loop
	vp->resume();
	delete event;
}

void Daemon::start_my_vps() {

	for (int i = 0; i < num_vps; ++i) {
		if (vps[i]->get_id() == 0) { // this is the main vp ...
			// so I have to tell AnahyVM, and not to start
			// the VP in a new thread
			AnahyVM::set_main_vp(vps[i]);
		}
		else {
			vps[i]->start();
		}
	}
}

void Daemon::stop_my_vps() {
	// stop my vps
	for (int i = 0; i < num_vps; ++i) {
		if (vps[i]->get_id() > 0) { // not main VP
			vps[i]->stop();
		}
	} 
}

void Daemon::run() {
	VPEvent* event;
	Job* job;

	start_my_vps();

	pthread_mutex_lock(&mutex);
	while (true) {
		if (vps_waiting.size() == num_vps) {
			// all vps are waiting

			job = AnahyVM::blocking_get_job();

			if (!job) { // time to stop
				printf("Daemon %d: will stop!\n", id);
				while (vps_waiting.empty() == false) {
					answer_oldest_vp_waiting(NULL);
				}
				pthread_mutex_unlock(&mutex);
				break;
			}
			else {
				answer_oldest_vp_waiting(job);
			}
		}
		else {
			if (event_queue.empty()) {
				log << "Waiting for events...\n";
				pthread_cond_wait(&event_cond, &mutex);
			}
			else {
				while (event_queue.empty() == false) {
					event = event_queue.front();
					event_queue.pop();
					pthread_mutex_unlock(&mutex);
					handle_event(event);
					pthread_mutex_lock(&mutex);
				}
			}
		}
	}

	stop_my_vps();
}

void Daemon::handle_get_job(VPEvent* event) {
	VirtualProcessor* vp;
	Job* job;
	VPEvent* unhandled;

	log << "GetJob event received from VP "
		<< event->get_sender()->get_id() << " ... ";

	job = AnahyVM::get_job(NULL);
	if (!job) {
		log << "No job. VP is gonna wait.\n";
		event->get_sender()->say("waiting...");
		vps_waiting.push(event);	
	}
	else {
		VirtualProcessor* vp = event->get_sender();
		vp->set_current_job(job);
		vp->resume();
		log << "Job assigned!\n";
		delete event;				
	}
}

void Daemon::handle_new_job(VPEvent* event) {
	VirtualProcessor* vp;
	Job* job;
	VPEvent* unhandled;

	log << "NewJob event received from VP "
		<< event->get_sender()->get_id() << " ... ";

	if (vps_waiting.empty()) {
		AnahyVM::post_job(event->get_job(), false);
		log << "Job posted.\n";
		delete event;
	}
	else {
		AnahyVM::post_job(event->get_job(), true);
		unhandled = vps_waiting.front();
		vps_waiting.pop();
		vp = unhandled->get_sender();
		vp->set_current_job(event->get_job());
		vp->resume();
		log << "Bypassing AnahyVM! Job assigned to VP " << vp->get_id() << "\n";
		delete event;
		delete unhandled;
	}
}

void Daemon::handle_end_of_job(VPEvent* event) {
	VirtualProcessor* vp;
	Job* job;
	VPEvent* unhandled;

	log << "EndOfJob event received from VP "
		<< event->get_sender()->get_id() << "\n";
}

void Daemon::handle_destroy_job(VPEvent* event) {
	VirtualProcessor* vp;
	Job* job;
	VPEvent* unhandled;
	
}

void Daemon::handle_event(VPEvent* event) {

	switch (event->get_type()) {
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
			handle_destroy_job(event);
			break;
		default:
			log << "Unknown event...\n";
			fprintf(stderr, "Unknown event...\n");
			exit(1);
	}
}

Daemon::Daemon(int n_vps) : num_vps(n_vps) {
	id = instances++;
	stop_signal = false;

	char file_name[20];
	sprintf(file_name, "logs/Daemon%d.log", id);
	log.open(file_name, ifstream::out);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&event_cond, NULL);

	// create vps
	for (int i = 0; i < num_vps; ++i) {
		vps.push_back(new VirtualProcessor(this));
	}
}

Daemon::~Daemon() {
	log << "Daemon object deleted!\n";
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&event_cond);
	for (int i = 0; i < num_vps; ++i) {
		delete vps[i];
	}
	vps.clear();
	log.close();
}

void Daemon::push_new_event(VPEvent* event) {
	pthread_mutex_lock(&mutex);
	event_queue.push(event);
	pthread_cond_signal(&event_cond); // wake daemon
	pthread_mutex_unlock(&mutex);
}

// called from a vp thread
void Daemon::get_job(VirtualProcessor* sender, Job* job) {
	push_new_event(new VPEvent(GetJob, sender, job));
	sender->block(); // block vp thread
}

// called from a vp thread
void Daemon::new_job(VirtualProcessor* sender, Job* job) {
	push_new_event(new VPEvent(NewJob, sender, job));
}

// called from a vp thread
void Daemon::end_of_job(VirtualProcessor* sender, Job* job) {
	push_new_event(new VPEvent(EndOfJob, sender, job));
}

// called from a vp thread
void Daemon::destroy_job(VirtualProcessor* sender, Job* job) {
	push_new_event(new VPEvent(DestroyJob, sender, job));
}

// called from AnahyVM
void Daemon::start() {
	log << "Starting Daemon...\n";
	// create my own thread
	pthread_create(&thread, NULL, run_daemon, this); 
}

// called from AnahyVM
void Daemon::stop() {
	// join my own thread
	pthread_join(thread, NULL);
	log << "Daemon stopped!\n";
}