#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "athread.h"
#include "graph_operation.h"

/* environment variables */

pthread_t main_thread, scheduler_daemon;
pthread_t* vp_array;
pthread_cond_t 	cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int num_vps;
unsigned long Job::counter = 0;
bool env_is_running;
std::list<Job*> root_threads;

/* environment variables */

Job::Job() {
        printf("New thread!\n");
        id = counter++;
}

Job::Job(pfunc func, void* job_data) {
        function = func;
        data = job_data;
        id = counter++;
}

void Job::set_job(Job* j) {
        job = j;
}

Job* Job::get_job() const {
        return job;
}

void Job::set_parent(Job* p) {
        parent = p;
}

Job* Job::get_parent() const {
        return parent;
}

void Job::set_creator(pthread_t* c) {
        creator = c;
}

Job* Job::get_creator() const {
        return creator;
}

ThreadState Job::get_state() const {
    return state;
}

unsigned long Job::get_id() const {
        return id;
}

void Job::run() {
       (*pfunc)(data);
}

Job* get_job(Job* starting_point) {
	int ret_code;
	
	/* STUB  */			return NULL; 		/* STUB  */
	
	if (!starting_point) {
		/* if no starting_point is defined 
			search for a job from the root */
	}
	else {
		
	}
}

void notify_completion(Job* job) {
	printf("Job %lu is done!", job->get_id());
}

void* scheduler_function(void* args) {
	int ret_code;
	pthread_t self = pthread_self();

	printf("Scheduler running on thread %lu!\n", (unsigned long) self);
	
	if (args) {
		// treat args ...
	}
	
	while (true) {
		ret_code = pthread_mutex_lock(&mutex);
		checkResults("pthread_mutex_lock", ret_code);
		
		if (!env_is_running) {
			ret_code = pthread_mutex_unlock(&mutex);
			checkResults("pthread_mutex_unlock", ret_code);
			break;
		}
		
		if (!graph_operations.empy()) {
			GraphOperation* operation = graph_operations.front();
			graph_operations.pop();
			ret_code = pthread_mutex_unlock(&mutex);
			checkResults("pthread_mutex_unlock", ret_code);
			operation->commit();
		}
		else {
			// wait for new graph operations...
		}
	}
}

void* vp_function(void* args) {
	Job* job;
	int ret_code;
	pthread_t self = pthread_self();

	printf("VP %lu running!\n", (unsigned long) self);
	
	while (true) {
		ret_code = pthread_mutex_lock(&mutex);
		
		if (!env_is_running) {
			ret_code = pthread_mutex_unlock(&mutex);
			break;
		}
		
		job = get_job(NULL);
		if (job) {
			ret_code = pthread_mutex_unlock(&mutex);
			job->run();
			notify_completion(job);
		}
		else { // go wait
			puts("waiting for new jobs or the end of the program...");
			ret_code = pthread_cond_wait(&cond, &mutex);
			// lock acquired... do something?
			ret_code = pthread_mutex_unlock(&mutex);
		}
	}
	
	printf("VP %lu shutting down...\n", (unsigned long) self);
	return NULL;
}

void aInit(int* _argc, char*** _argv) {
	main_thread = pthread_self();
	
	int c;
        while(1) {
            if((c = getopt(argc, argv, "v:s:m:")) == -1)
                break;
            switch(c) {
                case 'v':
                    num_vps = strtol(optarg, NULL, 10);
                    if(num_vps < 1) {
                        printf("Invalid number of vps, assuming 1 vp!\n");
                        num_vps = 1;
                    }
                    break;
                case 's':
                    //set scheduler (our problem)
                    break;
                case 'm':
                    /* set mode to executing and the 
                     * frequency of the cores (Alan says my problem)
                     */
                    break;
                default:
                    //set default state and frequency of the cores
                    break;
            }
            
        }
	
	printf("Anahy started: %d VPs running.\n", num_vps);
	env_is_running = true;
	
	// allocate and launch VPs
	vp_array = (pthread_t*) malloc(num_vps * sizeof(pthread_t));
	for (int i = 0; i < num_vps; i++) {
		pthread_create(&vp_array[i], NULL, vp_function, NULL);
	}
}

void aTerminate() {
	printf("Shutting Anahy down...\n");
	int ret_code;
	ret_code = pthread_mutex_lock(&mutex);
	env_is_running = false;
	ret_code = pthread_cond_broadcast(&cond);
	ret_code = pthread_mutex_unlock(&mutex);
	
	for (int i = 0; i < num_vps; i++) {
		pthread_join(vp_array[i], NULL);
	}
}

int athread_create(Job* handle, athread_attr_t* attr, pfunc function,
void* args) {
							
	pthread_t self = pthread_self();
	// handle->set_job(new Job(function, args));
	// handle->set_creator(self);
	// handle->set_parent(/* e agora?? */)

	// who am I ?
	if (self == main_thread) {
		// main thread

		printf("New thread created from main()\n");
	}
	else {
		// VP thread
		
		printf("New thread created by VP %lu\n", (unsigned long) self);
	}
}

int athread_join(Job* handle, void** result) {
	ThreadState st = handle->get_state();
	pthread_t self = pthread_self();
	
	// who am I ?
	if (self == main_thread) {
		// main thread

		printf("Thread joined from main()\n");
	}
	else {
		// VP thread
		
		printf("Thread joined by VP %lu\n", (unsigned long) self);
	}
}