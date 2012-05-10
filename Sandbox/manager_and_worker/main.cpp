#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include "Manager.h"
#include "WorkerEvent.h"

int main(int argc, char const *argv[])
{
	srand(time(NULL));

	Manager m(4);
	m.start();

	m.stop();

	return 0;
}