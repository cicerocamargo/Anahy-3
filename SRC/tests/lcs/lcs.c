#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <athread.h>

typedef struct {
    char *_a;
    char *_b;
    char *_out;
} thr_param_t;

int match(char *a, char *b, char *out, int dep) {
    	int longest = 0;
		if (!a || !b) return 0;
		if (!*a || !*b) {
			if (dep <= longest) return 0;
			out[ longest = dep ] = 0;
			return 1;
		}
 
		if (*a == *b)
			return match(a + 1, b + 1, out, dep + 1) && (out[dep] = *a);
 
		return	match(a + 1, b + 1, out, dep) + 
			match(strchr(a, *b), b, out, dep) +
			match(a, strchr(b, *a), out, dep);
}

void lcs(thr_param_t* in) {

    thr_param_t param = *((thr_param_t *) in);
    
    char *a;
    char *b;
    char *out;
    
    a = param._a;
    b = param._b;
    out = param._out;

	int dep = 0;
	
	int foo = match(a, b, out, dep);
	
	//printf("%s\n", out);
}
 
int main(int argc, char** argv) {

    aInit(&argc, &argv);
    
    int sizeIn1, sizeIn2, sizeOut, iterators;

    scanf("%d %d %d %d", &sizeIn1, &sizeIn2, &sizeOut, &iterators);
    
    thr_param_t param[iterators];
    athread_t thr[iterators];
    
    int i;

    for(i=0; i < iterators; i++) {

        char *a = malloc(sizeIn1*sizeof(char));
        char *b = malloc(sizeIn2*sizeof(char));;
        char *out = malloc(sizeOut*sizeof(char));

        scanf("%s",a);
        scanf("%s",b);
    
        param[i]._a = a;
        param[i]._b = b;
        param[i]._out = out;

        athread_create(&thr[i], NULL,(void*)& lcs, (void*) param); 

        athread_join(thr[i], NULL);

        free(a);
        free(b);
        free(out);
    }

    aTerminate();
    return 0;
}
