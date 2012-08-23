#ifndef JOBGRAPH_H
#define JOBGRAPH_H

#include <list>

using namespace std;

class Job;
class JobId;

class JobGraph {
	list<Job*> local_jobs;

public:
	JobGraph();
	~JobGraph();

	void insert(Job* job);
	void erase(Job* job);
	Job* find_a_ready_job(Job* starting_job, bool steal_job);
};

#endif