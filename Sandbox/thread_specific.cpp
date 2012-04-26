#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_key_t current_vp_key;

typedef void*(*par_func)(void*);

long fib(long n) {
	return n < 2 ? n : fib(n-1) + fib(n-2);
}

class VirtualProcessor {
public:
   static int counter;
   int id;
	
	VirtualProcessor() {
		id = counter++;
	}

	void* runs_user_func(par_func func, void* args) {
		return func(args);
	}
};

int VirtualProcessor::counter = 0;

void* fibonacci(void* args) {
	VirtualProcessor* vp;
	vp = (VirtualProcessor*) pthread_getspecific(current_vp_key);
	printf("I'm being executed by processor %d\n", vp->id);
	int x = fib(*(int*) args);
	return NULL;
}

void* run_vp(void *vp_obj) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;

	int ret_code = pthread_setspecific(current_vp_key, (void *) vp);
	if ( ret_code <  0) {
		printf("pthread_setspecific failed, thread %lu, error code %d", pthread_self(), ret_code);
		pthread_exit((void *)&ret_code);
	}

	int n = 40;
	return vp->runs_user_func(fibonacci, (void*)&n);
}

void call_vp_destructor(void *vp_obj) {
   VirtualProcessor* vp = (VirtualProcessor*) vp_obj;
   printf("Destroying VP %d\n", vp->id);
   delete vp;
}

int main(int argc, char** argv) {
	const int num_vps = atoi(argv[1]);
	int ret_code;
	VirtualProcessor** vp_array;
	pthread_t thread_array[num_vps];

	if ((ret_code = pthread_key_create(&current_vp_key, call_vp_destructor)) < 0) {
			printf("pthread_key_create failed, error code %d", ret_code);
			exit(1);
	}

	// create VPs
	vp_array = (VirtualProcessor**) malloc(num_vps*sizeof(VirtualProcessor*));
	for (int i = 0; i < num_vps; i++) {
		vp_array[i] = new VirtualProcessor();
	}

	for (int i = 1; i < num_vps; i++) {
		pthread_create(&thread_array[i], NULL, run_vp, (void *)vp_array[i]);
	}

	run_vp((void*) vp_array[0]);
	call_vp_destructor((void*) vp_array[0]);

	for (int i = 1; i < num_vps; i++) {
		pthread_join(thread_array[i], NULL);
	}
	return 0;
}