/* a chave para acessar o objeto VirtualProcessor
 * armazenado na memória específica do thread pode
 * ser armazenada como um atributo DA CLASSE VirutalProcessor.
 * Neste caso, temos que fornecer métodos de classe para
 * inicializarmos a chave e obter esta chave nas funçoes da API
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef void*(*par_func)(void*);

// para gerar carga sintética de trabalho
long fib(long n) {
	return n < 2 ? n : fib(n-1) + fib(n-2);
}

// protótipo exigido por pthread_key_create()
void call_vp_destructor(void *vp_obj);	

// classe meramente ilustrativa!
// não substitui nada no código de produção!
class VirtualProcessor {
	
public:
	static pthread_key_t key;
	static int counter;
	int id;
	
	VirtualProcessor() {
		id = counter++;
	}

	static void init_pthread_key() {
		int ret_code;
		if ((ret_code = pthread_key_create(&key, call_vp_destructor)) < 0) {
			printf("pthread_key_create failed, error code %d", ret_code);
			exit(1);
		}
	}

	static pthread_key_t get_pthread_key() {
		return key;
	}

	void* run_user_func(par_func func, void* args) {
		return func(args);
	}
};

// inicialização dos atributos de classe
int VirtualProcessor::counter = 0;
pthread_key_t VirtualProcessor::key;

// para testar pthread_GETspecific()
void* fibonacci(void* args) {
	VirtualProcessor* vp;
	
	vp = (VirtualProcessor*) pthread_getspecific(VirtualProcessor::get_pthread_key());
	
	printf("I'm being executed by processor %d\n", vp->id);
	int x = fib(*(int*) args);
	return NULL;
}

// para testar pthread_SETspecific()
void* run_vp(void *vp_obj) {
	VirtualProcessor* vp = (VirtualProcessor*) vp_obj;

	int ret_code = pthread_setspecific(VirtualProcessor::get_pthread_key(), (void *) vp);
	
	if ( ret_code <  0) {
		printf("pthread_setspecific failed, thread %lu, error code %d", (long)pthread_self(), ret_code);
		pthread_exit((void *)&ret_code);
	}

	// chama a função do usuário, que usa pthread_getspecific()
	int n = 40;
	return vp->run_user_func(fibonacci, (void*)&n);
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

	// inicializa a chave na classe VirtualProcessor
	VirtualProcessor::init_pthread_key();

	// cria os VPs
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