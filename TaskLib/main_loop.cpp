#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <list>

int num_vps, num_tasks, vps_waiting=0;

using namespace std;

bool					program_finished = false;
pthread_cond_t 	cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

list<int> task_list;

int new_task() {
	int task = (rand() % 3) + 3; // 3, 4 or 5
	printf("New task arrived with cost %d!!\n", task);
	return task; 
}

void run_task(int task, int vp_id) {
	printf("VP %d running a task with cost %d\n", vp_id, task);
	sleep(task);
}

int get_task() {
	int task = 0;
	
	if (!task_list.empty()) {
		task = task_list.front();
		task_list.pop_front();
	}

	return task;
}

void update_graph(int task) {
	int ret_code;
	if (task == 5) {
		ret_code = pthread_mutex_lock(&mutex);
		task_list.push_back(new_task());
		if (vps_waiting) {
			pthread_cond_broadcast(&cond);
		}
		ret_code = pthread_mutex_unlock(&mutex);
	}
}

void *vp_func(void *arg) {
	int vp_id = *((int*) arg);
	int ret_code;
	int task;
	
	printf("VP %d running on thread %ld.\n", vp_id, (long)pthread_self());
	for (;;) {
		ret_code = pthread_mutex_lock(&mutex);
		
		if (program_finished) {
			ret_code = pthread_mutex_unlock(&mutex);
			break;
		}
		else { // program is running
			task = get_task();
			
			if (task) {
				ret_code = pthread_mutex_unlock(&mutex);
				run_task(task, vp_id);
				update_graph(task);
			}
			else {
				vps_waiting++;
				if (vps_waiting == num_vps) {
					printf("I'm VP %d and I say the game is over! muá-ha-ha-ha!!!\n", vp_id);
					program_finished = true;
					pthread_cond_broadcast(&cond);
					ret_code = pthread_mutex_unlock(&mutex);
				}
				else {
					printf("VP %d blocked!\n", vp_id);
					ret_code = pthread_cond_wait(&cond, &mutex);
					vps_waiting--;
					ret_code = pthread_mutex_unlock(&mutex);
				}
			}
		}
	}
	return NULL;
}


int main(int argc, char** argv) {
	srand(time(NULL));
	
	if(argc < 3) {
		fprintf(stderr, "usage: %s number_of_VPs number_of_tasks\n", argv[0]);
		return 1;
	}
	
	num_vps = atoi(argv[1]);
	num_tasks = atoi(argv[2]);
	pthread_t* vp_threads = (pthread_t*) malloc(num_vps*sizeof(pthread_t));
	
	for(int i = 0; i < num_tasks; ++i) {
		task_list.push_back(new_task());
	}
	
	printf("\nTask list: [");
	for(list<int>::iterator it = task_list.begin(); it != task_list.end(); it++) {
		list<int>::iterator aux = it;
		if(++aux == task_list.end()) {
			printf("%d", *it);
		}
		else printf("%d, ", *it);
	}
	printf("]\n\n");
	
	for(int i = 0; i < num_vps; ++i) {
		pthread_create(&vp_threads[i], NULL, vp_func, (void*) new int(i));
	}
	printf("Waiting for the VPs to finish work...\n");
	for(int i = 0; i < num_vps; ++i) {
		pthread_join(vp_threads[i], NULL);
	}
	
	return 0;
}