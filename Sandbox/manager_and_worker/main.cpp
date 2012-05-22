#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include "ManagerController.h"


int main(int argc, char const *argv[])
{
	srand(time(NULL));

	int managers = atoi(argv[1]);
	int workers_per_manager = atoi(argv[2]);
	int initial_work = atoi(argv[3]);

	ManagerController::init(managers, workers_per_manager, initial_work);
	// do stuff
	ManagerController::terminate();	

	return 0;
}