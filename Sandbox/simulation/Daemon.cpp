#include "Daemon"

class Job;
class VP;

// this method is called from a VP thread
void Daemon::find_a_ready_job(VP* sender) {
	pthread_mutex_lock(&mutex);
	
	running_list.remove(sender);
	waiting_list.insert(sender);
	pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);
	sender->block();
}

void Daemon::broadcast_null_job() {
	for (list<Job*>::iterator i = waiting_list.begin(); i != waiting_list.end(); ++i) {
		(*i)->set_current_job(NULL);
		(*i)->resume();
	}	
}

void Daemon::start_my_vps() {
	list<VirtualProcessor*>::iterator it;
	
	for (it = running_list.begin(); it != running_list.end(); ++it) {
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

void Daemon::run() {
 	//should_stop = false;
 	start_my_vps();

 	VirtualProcessor* vp;
 	Job* job;
 	pthread_mutex_lock(&mutex);
 	while (true) {
 		if(waiting_list.size() == num_vps) {
 			//all my vps are waiting, this is the end!
 			broadcast_null_job();
 			pthread_mutex_unlock(&mutex);
 			break;
 		}
 		else {
 			if(vps_waiting.size() == 0) {
 				pthread_cond_wait(&cond, &mutex);
 			}
 			else {
 				vp = vps_waiting.front();
 				vps_waiting.pop_front();
 				pthread_mutex_unlock(&mutex);
 				bool job_not_found = true;
 				list<VirtualProcessor*>::iterator it;
 				for(it = running_list.begin(); it != running_list.end(); it++) {

 					job = (*it)->get_local_job();
 					if(job) {
 						vp->set_current_job(job);
 						running_list.push_back(vp);
 						vp->resume();
 						job_not_found = false;
 						break;
 					}
 				}

 				pthread_mutex_lock(&mutex);
 				if(job_not_found) {
 					waiting_list.push_front(vp);
 					pthread_cond_wait(&cond, &mutex);
 				}
 			}
 		}
	}

	stop_my_vps();
}