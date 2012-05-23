#include <pthread.h>
#include <cstdlib>
#include <iostream>

using namespace std;

#define DISPLAY(a) std::cout << #a << ": " << (a) << std::endl

enum State { ready, running, finished };

State st = ready;

int fib_(int n) {
	return n < 2 ? n : (fib_(n-1) + fib_(n-2));
}

void* atomic_decrement(void* args) {
	fib_(30);

	int* i = ((int*) args);

	if (__sync_bool_compare_and_swap(&st, ready, running)) {
		cout << "VP " << (*i) << " vai executar o Job!" << endl;
	}
	return NULL;
}

int main(int argc, char const *argv[]) {
	int vps = atoi(argv[1]);
	const int num_threads = vps;

	pthread_t th_array[num_threads];
	

	for (int i = 0; i < num_threads; ++i) {
		pthread_create(&th_array[i], NULL, atomic_decrement, new int(i));
	}

	for (int i = 0; i < num_threads; ++i) {
		pthread_join(th_array[i], NULL);
	}

	
	return 0;
}