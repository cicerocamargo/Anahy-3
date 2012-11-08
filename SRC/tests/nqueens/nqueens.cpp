#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "../../AnahyVM.h"
#include "../../AnahyJob.h"
 
typedef struct {
    int *hist;
    int col;
    int n;
} thr_param_t; 
 
void* solve(void *arg) {

    thr_param_t param = *((thr_param_t *) arg);

    int i, j, col;
    
    int n;
    int *hist;
    
    hist = param.hist;
    n = param.n;
    col = param.col;
    
#	define attack(i, j) (hist[j] == i || abs(hist[j] - i) == col - j)
	for (i = 0, j = 0; i < n; i++) {
		for (j = 0; j < col && !attack(i, j); j++);
		if (j < col) continue;
 
		hist[col] = i;
		
		thr_param_t param1;

        AnahyJob job;
		
		param1.hist = hist;
		param1.n = n;
		param1.col = col+1;
		
        job.init(solve, &param1, NULL);

        AnahyVM::fork(&job);

        AnahyVM::join(&job, NULL);
	}
}
 
int main(int argc, char **argv) {

    AnahyVM::init(argc, argv);
    int n;
    
    n = atoi(argv[1]);
    
    thr_param_t param;
    
    int *vec = (int*) malloc(n*sizeof(int));
    
    param.hist = vec;
    param.n = n;
    param.col = 0;

    AnahyJob job;

    job.init(solve, &param, NULL);

    AnahyVM::fork(&job);

    AnahyVM::join(&job, NULL);
    
    AnahyVM::terminate();
    
    free(vec);

    return 0;
}
