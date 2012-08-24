#include "JobGraph.h"
#include "JobId.h"
#include "Job.h"
#include <cstdio>

JobGraph::JobGraph() {

}

JobGraph::~JobGraph() {
	local_jobs.clear();
}

void JobGraph::insert(Job* job) {
	local_jobs.push_back(job);
}

void JobGraph::erase(Job* job) {
	list<Job*>::iterator it;

	for (it = local_jobs.begin(); it != local_jobs.end(); ++it) {
		if (*it == job) {
			it = local_jobs.erase(it);
			break;
		}
	}
}

Job* JobGraph::find_a_ready_job(Job* starting_job, bool steal_job) {
	set<Job*> children;
	set<Job*>::iterator it_child;
	
	if (steal_job) {
		list<Job*>::reverse_iterator rit;
		for(rit = local_jobs.rbegin(); rit != local_jobs.rend(); ++rit) {
			if (!(*rit)->get_vp_thief() && (*rit)->compare_and_swap_state(ready, running)) {
				return *rit;
			}
		}

	}
	else {
		list<Job*>::iterator it;
		if (starting_job && !starting_job->get_vp_thief()) {

			children = starting_job->get_children();
			if (!children.empty()) {
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
			for (it = local_jobs.begin(); it != local_jobs.end(); ++it) {
				if (!(*it)->get_vp_thief() && (*it)->compare_and_swap_state(ready, running)) {
					return *it;
				}
			}
		}
	}
	return NULL;
}