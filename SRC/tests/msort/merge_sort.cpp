#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <athread.h>
#include <sys/time.h>

/* Estrutura de dados que serão manipulados pelas threads */
typedef struct {
    int *vec;
    int vecSize;
    int pos;
    int tam;
    int maxthread;
    int numofThreads;
} thr_param_t;


void mergeSort(thr_param_t *dados);

int main(int argc, char** argv) {

    aInit(&argc, &argv);
    
    int max, i, tamanho, maxthread;
    
    scanf("%d", &max);
    
    tamanho = max;
    maxthread = 64;
    
    int *vetor = malloc(max * sizeof (int));

    for (i = 0; i < max; i++)
        scanf("%d", &vetor[i]);

    thr_param_t param;
    param.vec = vetor;
    param.pos = 0;
    param.vecSize = tamanho - 1;
    param.tam = tamanho;
    param.maxthread = maxthread;
    param.numofThreads = 0;
    
    athread_t thread;
    athread_create(&thread, NULL, (void*) & mergeSort, (void*) & param);
    athread_join(thread, NULL);

    aTerminate();
       
    return 0;
}

void mergeSort(thr_param_t *dados) {

    thr_param_t parametros = *((thr_param_t *) dados);
    
    int *vec,vecSize,pos,tamanho,maxthread;
    
    vec = parametros.vec;
    vecSize = parametros.vecSize;
    pos = parametros.pos;
    tamanho = parametros.tam;
    maxthread = parametros.maxthread;
    
    int mid;
    int i;
    thr_param_t param1, param2;
    int tam;
    tam = (vecSize - pos) + 1;
    if (tam > 3) {
        mid = (pos + vecSize) / 2;
        param1.vec = vec;
        param1.pos = pos;
        param1.vecSize = mid;
        param1.tam = tamanho;
        param1.maxthread = maxthread;
        param2.vec = vec;
        param2.pos = mid + 1;
        param2.vecSize = vecSize;
        param2.tam = tamanho;
        param2.maxthread = maxthread;
	param1.numofThreads = parametros.numofThreads+1;
	param2.numofThreads = parametros.numofThreads+2;
        if (parametros.numofThreads <= maxthread) {
	    athread_t thread1,thread2;
            athread_create(&thread1, NULL, (void*) & mergeSort, (void*) & param1);
            athread_create(&thread2, NULL, (void*) & mergeSort, (void*) & param2);
            athread_join(thread1, NULL);
            athread_join(thread2, NULL);
        } else {
            mergeSort((void*)&param1);
            mergeSort((void*)&param2);
        }
        //SORT
        int j, k, size, vecLen;
        size = vecSize - pos + 1;
        int *v_temp = malloc(tamanho * sizeof (int));

        if (v_temp == NULL) {
            printf(" ERRO - Nao foi possivel alocar memoria para vetor temporario");
            exit(0);
        }
        i = pos;
        j = mid + 1;
        k = pos;
        while ((i <= mid) && (j <= vecSize)) {
            if (vec[i] > vec[j]) {
                v_temp[k] = vec[j];
                k++;
                j++;
            } else {
                v_temp[k] = vec[i];
                k++;
                i++;
            }
        }
        for (i = i; i <= mid; i++, k++)
            v_temp[k] = vec[i];
        for (j = j; j <= vecSize; j++, k++)
            v_temp[k] = vec[j];
        for (i = pos; i <= vecSize; i++)
            vec[i] = v_temp[i];

        free(v_temp);

    } else {
        int aux, ordenado = 1;
        do {

            for (i = pos; i < vecSize; i++) {
                ordenado = 1;
                if (vec[i] > vec[i + 1]) {
                    aux = vec[i];
                    vec[i] = vec[i + 1];
                    vec[i + 1] = aux;
                    ordenado = 0;
                }
            }
        } while (ordenado == 0);
    }
}
