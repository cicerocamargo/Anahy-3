#include "ourlist.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <pthread.h>

#define NUM_THREADS 8
#define NUM_OPERATIONS 5000000

mList<int> l;
int numElements = 0;
pthread_mutex_t mutex;
int threadCounter = 0;

typedef enum {
	mListPushFront = 0,
	mListPushBack,
	mListPopFront,
	mListPopBack
} mListOperation;

void* foo(void* arg) {
	int threadId;
	pthread_mutex_lock(&mutex);
	threadId = threadCounter++;
	pthread_mutex_unlock(&mutex);
	printf("thread %d\n", threadId);

	for (int i = 0; i < NUM_OPERATIONS; ++i) {
		int op = rand() % 2;
		pthread_mutex_lock(&mutex);
		switch (op) {
			case mListPushFront:
				l.push_front(threadId);
				numElements++;
				break;
			case mListPushBack:
				l.push_back(threadId);
				numElements++;
				break;
			case mListPopFront:
				if (l.empty() == false) {
					l.pop_front();
					numElements--;
				}
				break;
			case mListPopBack:
				if (l.empty() == false) {
					l.pop_back();
					numElements--;
				}
				break;
			default:
				fprintf(stderr, "Unknown operation!\n");
				abort();
				break;
			}
		pthread_mutex_unlock(&mutex);	
	}
}


int main(int argc, char const *argv[])
{
	srand(time(NULL));

	assert(l.empty());
	assert(l.size() == 0);

	int val;
	l.push_back(val);
	assert(!l.empty());
	assert(l.size() == 1);
	assert(l.front() == val);
	assert(l.back() == val);

	l.pop_back();
	assert(l.empty());
	assert(l.size() == 0);

	l.push_front(val);
	assert(!l.empty());
	assert(l.size() == 1);
	assert(l.front() == val);
	assert(l.back() == val);

	l.pop_front();
	assert(l.empty());
	assert(l.size() == 0);

	pthread_mutex_init(&mutex, NULL);

	pthread_t thread_array[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&thread_array[i], NULL, foo, NULL);
	}
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(thread_array[i], NULL);
	}

	assert(l.size() == numElements);
	l.print();

	pthread_mutex_destroy(&mutex);
	return 0;
}