#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "../../AnahyVM.h"
#include "../../AnahyJob.h"

using namespace std;

typedef struct {
	unsigned long int m, n, result;
} thr_param_t;

void* parComb(void *arg) {

    thr_param_t param = *((thr_param_t *) arg);

    unsigned long int res;
    
    thr_param_t param1, param2;
    
    AnahyJob threadRamoEsq, threadRamoDir;
    
    if(param.n == 1)
        res = param.m;
    else if(param.m == param.n)
        res = 1;
    else if(param.m > param.n) {
        
        param1.m = param.m - 1;
        param1.n = param.n - 1;
        
        param2.m = param.m - 1;
        param2.n = param.n;
        
        threadRamoEsq.init(parComb, &param1, NULL);
        threadRamoDir.init(parComb, &param2, NULL);
        
        AnahyVM::fork(&threadRamoEsq);
        AnahyVM::fork(&threadRamoDir);

        AnahyVM::join(&threadRamoEsq, NULL);
        AnahyVM::join(&threadRamoDir, NULL);
        
        res = param1.result + param2.result;
    }
    param.result = res;
}

int main(int argc, char** argv) {
    
    AnahyVM::init(argc, argv);

    unsigned long int m, n;

    m = atoi(argv[1]);
    n = atoi(argv[2]);    
    
    AnahyJob job;

    thr_param_t param;

    param.result = 0;
    param.m = m;
    param.n = n;
      
    job.init(parComb, &param, NULL);

    AnahyVM::fork(&job);

    AnahyVM::join(&job, NULL);
    
    AnahyVM::terminate();
    return 0;
}
