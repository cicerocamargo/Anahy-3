#include "../VirtualProcessor.h"
#include "../Daemon.h"

int main(int argc, char const *argv[]) {
	VirtualProcessor* vp =  new VirtualProcessor(NULL, pthread_self());
	return 0;
}