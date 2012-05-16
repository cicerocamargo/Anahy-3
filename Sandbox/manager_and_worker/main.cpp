#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include "ManagerController.h"


int main(int argc, char const *argv[])
{
	srand(time(NULL));

	int managers=2, workers_per_manager=4;
	ManagerController::init(managers, workers_per_manager);
	ManagerController::start();
	ManagerController::stop();
	ManagerController::terminate();	

	return 0;
}