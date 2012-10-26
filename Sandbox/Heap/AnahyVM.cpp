#include "AnahyVM.h"
#include "VirtualProcessor.h"
#include "AnahyJobAttributes.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "VirtualProcessor.h"

VirtualProcessor* AnahyVM::main_vp;
VirtualProcessor** AnahyVM::vp_array;
pthread_t* AnahyVM::thread_array;
pthread_key_t AnahyVM::key;
int AnahyVM::num_vps;
int AnahyVM::num_cpus;

// SYSTEM MANAGEMENT INTERFACE

void* AnahyVM::call_vp_run(void* vp_obj) {
	pthread_setspecific(key, vp_obj);
	((VirtualProcessor*)vp_obj)->run();
	return NULL;
}

void AnahyVM::call_vp_destructor(void* vp_obj) {}

void AnahyVM::parse_user_options(int argc, char **argv) {
	// if we have only two arguments and the second one asks for help or the version...
	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			AnahyVM::show_help();
			exit(0);
		} else if (strcmp(argv[1], "--version") == 0) {
			printf("\nAnahy %.1f (Beta) runtime.\n\n", VERSION);
			exit(0);
		}
	} 
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-v") == 0 && i+1 < argc) {
			// we have v and no more arguments
			int _num_vps = atoi(argv[++i]);
			num_vps = _num_vps > 1 ? _num_vps : 1;
			break;
		}
	}
}

void AnahyVM::show_help() {
	printf("\nOptions:\n");
	printf("	-h show this help message.\n");
	printf("	-v # set virtual processors to the environment;\n\t\t\t\tDefault is 1.\n");
	printf("\n");
}

//here the interface begins to be described	
void AnahyVM::init(int argc, char **argv) {
	int c;
	num_vps = 1; // In case the user doesn't specify a number

	AnahyVM::parse_user_options(argc, argv);

	pthread_key_create(&key, call_vp_destructor);

	// allocate an array of processor objects
	vp_array = (VirtualProcessor**)malloc(num_vps*sizeof(VirtualProcessor*));
	for(int i = 0; i < num_vps; i++) {
		vp_array[i] = new VirtualProcessor(); // create VP objects
	}

	// allocate an array of threads that will run these processors
	thread_array = (pthread_t*)malloc((num_vps-1)*sizeof(pthread_t));

	main_vp = vp_array[0];
	pthread_setspecific(key, main_vp);
	for (int i = 1; i < num_vps; i++) {
		pthread_attr_t* attr = NULL; // TO DO
		pthread_create(&thread_array[i-1], NULL, call_vp_run, vp_array[i]);
		
		#ifdef DEBUG

			printf("New thread created for VP %d\n", i);

		#endif
	}
}

void AnahyVM::terminate() {

	main_vp->run();// this allows the main VP to help the execution of
					// remaining jobs and the Daemon to know that the
					// main VP is also idle when there's no work

	for (int i = 1; i < num_vps; i++) {
		pthread_join(thread_array[i-1], NULL);
		
		#ifdef DEBUG

		printf("Joined VP %d's thread\n", i);

		#endif
	}

	// release memory
	free(thread_array);
	for(int i = 0; i < num_vps; i++) { delete vp_array[i]; }
	free(vp_array);
	pthread_key_delete(key);
}

void AnahyVM::_exit(void* value_ptr) {
	VirtualProcessor* current_vp = (VirtualProcessor*) pthread_getspecific(key);
	AnahyJob* job = current_vp->get_current_job();
	job->set_result(value_ptr);
}

void AnahyVM::fork(AnahyJob* job) {
	VirtualProcessor* vp = (VirtualProcessor*) pthread_getspecific(key);
	vp->fork_job(job);
}

void AnahyVM::join(AnahyJob* job, void** result) {
	VirtualProcessor* vp = (VirtualProcessor*) pthread_getspecific(key);

	void* temp = vp->join_job(job);

	if(result) {
		*result = temp;
	}
}

// USER ATTRIBUTES INTERFACE

void AnahyVM::attr_init(AnahyJobAttributes* attr) {
	AnahyJobAttributes* _attr = new AnahyJobAttributes();
	attr = _attr;
}

void AnahyVM::attr_setjobcost(AnahyJobAttributes* attr, JobCost cost) {
	attr->set_job_cost(cost);
}

int AnahyVM::attr_getjobcost(AnahyJobAttributes* attr) {
	return attr->get_job_cost();
}