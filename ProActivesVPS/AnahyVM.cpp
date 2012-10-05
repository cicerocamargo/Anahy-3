#include "include/AnahyVM.h"
#include "include/VirtualProcessor.h"
#include "include/JobAttributes.h"
#include <cstdio>
#include <cstdlib>

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
	Job* job = current_vp->get_current_job();
	job->set_retval(value_ptr);
}

void AnahyVM::create(athread_t* handle, athread_attr_t* attr, pfunc function, void* args) {

	VirtualProcessor* vp = (VirtualProcessor*) pthread_getspecific(key);
	JobAttributes* _attr;
	//we need that job's attributes has been initialized, because
	//we will need it to decrement the number of job's joins
	if (!attr) {
		_attr = new JobAttributes();
	} else {
		_attr = attr;
	}
	*handle = vp->create_new_job(function, _attr, args);
}

void AnahyVM::join(athread_t handle, void** result) {
	VirtualProcessor* vp = (VirtualProcessor*) pthread_getspecific(key);

	void* temp = vp->join_job(handle);

	if(result) {
		*result = temp;
	}
}

// USER ATTRIBUTES INTERFACE

void AnahyVM::attr_init(JobAttributes* attr) {
	JobAttributes* _attr = new JobAttributes();
	attr = _attr;
}

void AnahyVM::attr_setjobcost(JobAttributes* attr, JobCost cost) {
	attr->set_job_cost(cost);
}

void AnahyVM::attr_setjobjoins(JobAttributes* attr, int joins) {
	attr->set_num_joins(joins);
}

int AnahyVM::attr_getjobcost(JobAttributes* attr) {
	return attr->get_job_cost();
}

void* parf_func(...) {
	AnahyJobAttributes autoreleased_attr(...);
	AnahyJob autoreleased_job(func, args);
	autoreleased_job.set_attributes(autoreleased_attr);
	Anahy::fork(&autoreleased_job);

	// default NULL for the attributes
	AnahyJob autoreleased_job_without_attributes(func, args);
	Anahy::fork(&autoreleased_job_without_attributes);

	AnahyJobAttributes* attr = new AnahyJobAttributes(...);
	AnahyJob* allocated_job = new Job(func, args);
	allocated_job->set_attributes(attr);
	Anahy::fork(allocated_job);

	// bla bla bla ...

	Anahy::join(autoreleased_job);
	Anahy::join(autoreleased_job_without_attributes);
	return (void*) allocated_job; // this job can be joined in a outer scope
}