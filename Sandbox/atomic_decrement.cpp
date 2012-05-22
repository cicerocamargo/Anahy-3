#include <pthread.h>
#include <iostream>

#define DISPLAY(a) std::cout << #a << ": " << (a) << std::endl

int x = 4;
const int num_threads = x;

void* atomic_decrement(void* args) {
	if (__sync_bool_compare_and_swap(&x, x, x-1)) {
		if (__sync_bool_compare_and_swap(&x, 0, -1)) {
			std::cout << "zerei x!" << std::endl;
		}
	}
	else {
		std::cout << "Deu zebra :P" << std::endl;
	}
	return NULL;
}

int main(int argc, char const *argv[]) {
	pthread_t th_array[num_threads];
	DISPLAY(x);

	for (int i = 0; i < num_threads; ++i) {
		pthread_create(&th_array[i], NULL, atomic_decrement, NULL);
	}

	for (int i = 0; i < num_threads; ++i) {
		pthread_join(th_array[i], NULL);
	}

	DISPLAY(x);	
	return 0;
}