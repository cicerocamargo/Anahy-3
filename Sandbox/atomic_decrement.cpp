#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <semaphore.h>
#include <sys/time.h>

using namespace std;

unsigned int buffer;
unsigned int full_buffer;

pthread_mutex_t mutex;
sem_t bufferVazio, bufferCheio;

void* atomic_increment_cmps(void* args);
void* atomic_decrement_cmps(void* args);

void* atomic_increment_sem(void* args);
void* atomic_decrement_sem(void* args);

void* atomic_increment_mutex(void* args);
void* atomic_decrement_mutex(void* args);

int main(int argc, char **argv) {
	
	int loop_size, num_threads;
	char* control_type;
	
	num_threads = (atoi(argv[1])) / 2;
	loop_size = atoi(argv[2]);
	control_type = argv[3];
	
	pthread_t th_array_incr[num_threads];
	pthread_t th_array_decr[num_threads];
	
	buffer = 0;
	full_buffer = num_threads;
	
	switch(*control_type) {
		case 's':
			
				sem_init(&bufferCheio, 0, num_threads);
				sem_init(&bufferVazio, 0, 0);
			
				cout << "Executing increment using 'Semaphore'." << endl;
				for (int i = 0; i < num_threads; ++i)
					pthread_create(&th_array_incr[i], NULL, atomic_increment_sem, (void*)&loop_size);
				
				cout << "Executing decrement using 'Semaphore'." << endl;
				for (int d = 0; d < num_threads; ++d)
					pthread_create(&th_array_decr[d], NULL, atomic_decrement_sem, (void*)&loop_size);
				
				cout << "It will synchronize soon." << endl;
				
				for (int i = 0; i < num_threads; ++i) {
					pthread_join(th_array_incr[i], NULL);
				}
				
				for (int d = 0; d < num_threads; ++d) {
					pthread_join(th_array_decr[d], NULL);
				}
				
				cout << "Done "<< buffer << ".\n" << endl;
			break;
		case 'm':
				
				pthread_mutex_init(&mutex, NULL);
			
				cout << "Executing increment using 'Mutex'." << endl;
				for (int i = 0; i < num_threads; ++i)
					pthread_create(&th_array_incr[i], NULL, atomic_increment_mutex, (void*)&loop_size);
				
				cout << "Executing decrement using 'Mutex'." << endl;
				for (int d = 0; d < num_threads; ++d)
					pthread_create(&th_array_decr[d], NULL, atomic_decrement_mutex, (void*)&loop_size);
				
				cout << "It will synchronize soon." << endl;
				
				for (int i = 0; i < num_threads; ++i) {
					pthread_join(th_array_incr[i], NULL);
				}
				
				for (int d = 0; d < num_threads; ++d) {
					pthread_join(th_array_decr[d], NULL);
				}
				
				cout << "Done "<< buffer << ".\n" << endl;
			break;
		case 'c':
				cout << "Executing increment using 'Compare and Swap'." << endl;
				for (int i = 0; i < num_threads; ++i)
					pthread_create(&th_array_incr[i], NULL, atomic_increment_cmps, (void*)&loop_size);
				
				cout << "Executing decrement using 'Compare and Swap'." << endl;
				for (int d = 0; d < num_threads; ++d)
					pthread_create(&th_array_decr[d], NULL, atomic_decrement_cmps, (void*)&loop_size);
				
				cout << "It will synchronize soon." << endl;
				
				for (int i = 0; i < num_threads; ++i) {
					pthread_join(th_array_incr[i], NULL);
				}
				
				for (int d = 0; d < num_threads; ++d) {
					pthread_join(th_array_decr[d], NULL);
				}
				cout << "Done "<< buffer << ".\n" << endl;
			break;
		default:
			cout << "Operacao invalida, saindo!" << endl;
			exit(1);
	}
	return 0;
}

void* atomic_increment_cmps(void* args) {
	unsigned int loop_counter = *(unsigned int*)args;
	
	while(loop_counter) {
		if(__sync_bool_compare_and_swap(&buffer, full_buffer, buffer))
			;
		else {
			cout << "increment!" << buffer << endl;
			__sync_bool_compare_and_swap(&buffer, buffer, buffer + 1);
		}
		
		loop_counter--;
	}
	return NULL;
}

void* atomic_decrement_cmps(void* args){
	unsigned int loop_counter = *(unsigned int*)args;
	
	while(loop_counter) {
		if(__sync_bool_compare_and_swap(&buffer, 0, buffer))
			;
		else {
			cout << "decrement!" << buffer << endl;
			__sync_bool_compare_and_swap(&buffer, buffer, buffer - 1);
		}
		
		loop_counter--;
	}
	return NULL;
}

void* atomic_increment_sem(void* args) {
	unsigned int loop_counter = *(unsigned int*)args;
	
	while(loop_counter) {
		sem_wait(&bufferCheio);
		
		cout << "increment!" << buffer << endl;
		buffer++;
		sem_post(&bufferVazio);
		
		loop_counter--;
	}
	return NULL;
}

void* atomic_decrement_sem(void* args) {
	unsigned int loop_counter = *(unsigned int*)args;
	
	
	while(loop_counter) {
		sem_wait(&bufferVazio);
		
		cout << "decrement!" << buffer << endl;
		buffer--;
		
		sem_post(&bufferCheio);
		
		loop_counter--;
	}
	return NULL;
}

void* atomic_increment_mutex(void* args) {
	unsigned int loop_counter = *(unsigned int*)args;
	
	while(loop_counter) {
		pthread_mutex_lock(&mutex);
		
		cout << "increment!" << buffer << endl;
		buffer++;
		
		pthread_mutex_unlock(&mutex);
		
		loop_counter--;
	}
	return NULL;
}

void* atomic_decrement_mutex(void* args) {
	unsigned int loop_counter = *(unsigned int*)args;
	
	while(loop_counter) {
		pthread_mutex_lock(&mutex);
		
		cout << "decrement!" << buffer << endl;
		buffer--;
		
		pthread_mutex_unlock(&mutex);
		
		loop_counter--;
	}
	return NULL;
}