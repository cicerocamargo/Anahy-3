#include <pthread.h>
#include <list>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "athread.h"
#include "Daemon.h"
#include "VirtualProcessor.h"
#include "graph_operation.h"

/* environment variables */

class Job;

VirtualProcessor** vp_array;
pthread_t* vp_thread_array
pthread_cond_t 	cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
uint num_vps;

void* vp_run(void* vp_args) {
        VirtualProcessor* vp = (VirtualProcessor*) vp_args;
        vp->run();
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
	
	// allocate and launch VPs
	vp_array = (VirtualProcessor*) malloc(num_vps*sizeof(VirtualProcessor));
        
        Daemon* daemon = new Daemon();
        
        for(int i = 0; i < num_vps; i++) {
            vp_array[i] = new VirtualProcessor(&daemon);
        }
        
	for (int i = 0; i < num_vps; i++) {
		pthread_create(&vp_thread_array[i], NULL, run_vp,
(void*)vp_array[i]);
	}
}

void aTerminate() {
	printf("Shutting Anahy down...\n");
		
	for (int i = 0; i < num_vps; i++) {
		pthread_join(vp_thread_array[i], NULL);
	}
}

int athread_create(athread_t* thid, athread_attr_t* attr, pfunc function,
void* args) {
	
        VirtualProcessor* vp =
VirtualProcessor::get_vp_from_pthread(pthread_self());
        parent = vp->get_current_job();
        thid
        parent->add_child(thid);
	
}

int athread_join(athread_t thid, void** result) {
	
}