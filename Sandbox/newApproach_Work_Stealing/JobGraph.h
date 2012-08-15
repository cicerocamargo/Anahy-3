#ifndef JOBGRAPH_H
#define JOBGRAPH_H

#include <list>
#include <map>
using namespace std;

class Job;
class JobId;

class JobGraph {
	list<Job*> root_jobs;
	map<JobId, Job*> job_map;

public:
	JobGraph();
	~JobGraph();

	void insert(Job* job);
	void erase(Job* job);
	Job* find_a_ready_job(Job* starting_job);
};

#endif