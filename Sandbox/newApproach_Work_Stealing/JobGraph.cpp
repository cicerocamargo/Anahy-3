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

Job* JobGraph::find_a_ready_job(Job* starting_job, bool steal_job) {
	set<Job*> children;
	set<Job*>::iterator it_child;
	
	if (steal_job) {
		list<Job*>::reverse_iterator rit;
		for(rit = root_jobs.rbegin(); rit != root_jobs.rend(); ++rit) {
			if (!(*rit)->get_vp_thief() && (*rit)->compare_and_swap_state(ready, running)) {
				return *rit;
			}
			else {
				children = (*rit)->get_children();
				if (!children.empty()) {
					root_jobs.insert(root_jobs.begin(), children.begin(), children.end());
				}
				root_jobs.remove(*rit);
			}
		}

	}
	else {
		list<Job*>::iterator it;
		if (starting_job) {
			children = starting_job->get_children();
			if (!children.empty() || !starting_job->get_vp_thief()) {
				for (it_child = children.begin(); it_child != children.end(); ++it_child) {
					if(!(*it)->get_vp_thief() && (*it)->compare_and_swap_state(ready, running)) {
						return *it;
					}
				}
			}
			else {
				find_a_ready_job(NULL, NULL);
			}
		}
		else {
			for (it = root_jobs.begin(); it != root_jobs.end(); ++it) {
				if (!(*it)->get_vp_thief() && (*it)->compare_and_swap_state(ready, running)) {
					return *it;
				}
				else {
					children = (*it)->get_children();
					if (!children.empty()){
						root_jobs.insert(root_jobs.end(), children.begin(), children.end());
					}
					root_jobs.remove(*it);
				}
			}
		}
	}
	return NULL;
}