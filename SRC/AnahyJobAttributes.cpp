#include "AnahyJobAttributes.h"

AnahyJobAttributes::AnahyJobAttributes(bool smart) {
	_smart = smart;
	initialized = false;
	job_cost = NORMAL_COST;
}