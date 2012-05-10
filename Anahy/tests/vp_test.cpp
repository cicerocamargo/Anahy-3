#include "../VirtualProcessor.h"
#include "../Daemon.h"
#include <cstdio>


void* func(void* args) {
	puts("Running user code!");
}

int main(int argc, char const *argv[]) {
	// obtém a referência para o objeto Anahy
	AnahyVM* anahy = AnahyVM::get_instance_handler();

	// cria um job idiota no grafo do anahy
	anahy->create_dummy_job(func, NULL);

	// cria um daemon com dois processadores virtuais
	Daemon* daemon = new Daemon(2);

	// coloca o daemon para rodar
	daemon->start()

	// para o daemon
	daemon->stop();

	return 0;
}