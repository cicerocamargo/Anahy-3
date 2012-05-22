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

void Daemon::run() {
	VPEvent* event;
	VirtualProcessor* vp;

	// start my vps
	for (int i = 0; i < num_vps; ++i) {
		if (vps[i]->get_id() == 0) { // main vp
			AnahyVM::set_main_vp(vps[i]);
		}
		else {
			vps[i]->start();
		}
	}

	pthread_mutex_lock(&mutex);
	while (true) {
		if (stop_signal && vps_waiting.size() == num_vps) {
			// all vps are waiting, so this is the end ...
			while (vps_waiting.empty() == false) {
				event = vps_waiting.front(); // the related event
				vps_waiting.pop();
				vp = event->get_sender();
				vp->set_current_job(NULL);
				vp->resume();
				delete event;
			}
			pthread_mutex_unlock(&mutex);
			break;
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

	// stop my vps
	for (int i = 0; i < num_vps; ++i) {
		if (vps[i]->get_id() > 0) {
			vps[i]->stop();
		}
	} 
}

void Daemon::handle_event(VPEvent* event) {
	VirtualProcessor* vp;
	Job* job;
	VPEvent* unhandled;

	if (event->get_type() == GetJob) {
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
	else if (event->get_type() == NewJob) {
		log << "NewJob event received... ";
		if (vps_waiting.empty()) {
			AnahyVM::post_job(event->get_job());
			log << "Job posted.";
			delete event;
		}
		else {
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
	else {
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

// called from a vp thread
void Daemon::get_job(VirtualProcessor* sender) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new VPEvent(GetJob, sender, NULL));
	pthread_cond_signal(&event_cond); // wake daemon
	pthread_mutex_unlock(&mutex);
	sender->block(); // block vp thread
}

// called from a vp thread
void Daemon::new_job(Job* job) {
	pthread_mutex_lock(&mutex);
	event_queue.push(new VPEvent(NewJob, NULL, job));
	pthread_cond_signal(&event_cond);
	pthread_mutex_unlock(&mutex);
}

void Daemon::start() {
	log << "Starting Daemon...\n";
	// create my own thread
	pthread_create(&thread, NULL, run_daemon, this); 
}

void Daemon::signal_stop() {
	pthread_mutex_lock(&mutex);
	stop_signal = true;
	pthread_cond_signal(&event_cond);
	pthread_mutex_unlock(&mutex);
}

// called from AnahyVM
void Daemon::stop() {
	// join my own thread
	pthread_join(thread, NULL);
	log << "Daemon stopped!\n";
}