#ifndef JOBGRAPH_H
#define JOBGRAPH_H

#include <list>
#include <map>
using namespace std;

class Job;
class JobId;

struct Auxiliary_Cell
{
	struct Normal_Cell* next;
};

struct Normal_Cell
{
	bool first_last;
	Job* job;
	struct Auxiliary_Cell *next;
};

struct Cursor
{
	struct Normal_Cell* target, pre_cell;
	struct Auxiliary_Cell* pre_aux;
};


class JobGraph {


public:
	JobGraph();
	~JobGraph();

	void insert(Job* job);
	void erase(Job* job);
	Job* find_a_ready_job(Job* starting_job);
};

#endif