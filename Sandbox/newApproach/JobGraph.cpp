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
		//It's a root job so...
		root_jobs.push_back(job);
	}
	else {
		job_map[ job->get_id() ] = job;
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
	set<Job*> children;
	// temp code
	list<Job*> job_list = root_jobs;
	list<Job*>::iterator it;
	set<Job*>::iterator it_child;

	if(starting_job) {
		children = starting_job->get_children();
		if (!children.empty()) {
			for (it_child = children.begin(); it_child != children.end(); ++it_child) {
				if ((*it)->compare_and_swap_state(ready, running)) {
					return *it;
				}
			}
		}
		else {
			printf("Graph: Precisou chegar aqui\n\n");
			find_a_ready_job(NULL);
		}
	}
	else {

		for (it = job_list.begin(); it != job_list.end(); ++it) {
			if ((*it)->compare_and_swap_state(ready, running)) {
				return *it;
			}
			else {
				children = (*it)->get_children();
				if (!children.empty()){
					job_list.insert(job_list.end(), children.begin(), children.end());
				}
				it = job_list.erase(it);
			}
		}
	}
	return NULL;
}