#include "JobGraph.h"
#include "JobId.h"
#include "Job.h"
#include <cstdio>

JobGraph::JobGraph() {

}

JobGraph::~JobGraph() {
	root_jobs.clear();
	job_map.clear();
}

void JobGraph::insert(Job* job) {

	if (!job->get_parent()) {
		//It's a root job, so...
		root_jobs.push_back(job);
	}
	else {
		typedef pair<JobId, Job*> job_pair;

		job_map.insert(job_pair(job->get_id(), job));
		//job_map[ job->get_id() ] = job;
	}
}

void JobGraph::erase(Job* job) {
	JobId id = job->get_id();

	if (!job->get_parent()) {
		// it's a root job
		list<Job*>::iterator it;
		for (it = root_jobs.begin(); it != root_jobs.end(); ++it) {
			if (*it == job) {
				it = root_jobs.erase(it);
				break;
			}	
		}
	}
	else {
		//  it's NOT a root job
		job_map.erase(job->get_id());
	}

	// delete job;
}

Job* JobGraph::find_a_ready_job(Job* starting_job) {
	map<JobId, Job*>::iterator it;
	set<Job*>::iterator it_child;
	set<Job*> children;	
	
	if (starting_job) {
		children = starting_job->get_children();
		if (!children.empty()) {
			for (it_child = children.begin(); it_child != children.end(); ++it_child) {
				if ((*it_child)->compare_and_swap_state(ready, running)) {
					return *it_child;
				}
			}
		}
		else {
			//printf("Graph: It tried run a child, but I couldn't do it.\n\tSearching a root job.\n");
			find_a_ready_job(NULL);
		}
	}
	else {
		for (it = job_map.begin(); it != job_map.end(); ++it) {
			if (((*it).second)->compare_and_swap_state(ready, running)) {
				return (*it).second;
			}
		}
	}
	return NULL;
}

Job* JobGraph::find_a_root_ready_job(Job* starting_job) {
	list<Job*>::iterator it;
	set<Job*> children;

	for (it = root_jobs.begin(); it != root_jobs.end(); ++it) {
		if ((*it)->compare_and_swap_state(ready, running)) {
			return *it;
		}
	}
	return NULL;
}