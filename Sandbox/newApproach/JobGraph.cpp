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
	list<Job*>::iterator it;
	set<Job*>::iterator it_child;
	//HERE ERROR - HERE ERROR - HERE ERROR
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
			printf("Graph: It tried run a child, but I couldn't do it.\n\tSearching a root job.\n");
			find_a_ready_job(NULL);
		}
	}
	else {
		printf("Graph: Ok, no one starting_job has been gave me.\n\tI'll find from the root.\n");
		for (it = root_jobs.begin(); it != root_jobs.end(); ++it) {
			if ((*it)->compare_and_swap_state(ready, running)) {
				return *it;
			}
			else {
				children = (*it)->get_children();
				if (!children.empty()){
					root_jobs.insert(root_jobs.end(), children.begin(), children.end());
				}
				it = root_jobs.erase(it);
			}
		}
	}
	printf("Mimimi, no on job is ready, I'll give you a NULL.\n");
	return NULL;
}