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

/* I don't know if what I'm doing in this method is right, because it's 3 AM muda fucka LOL*/
Job* JobGraph::find_a_ready_job(Job* starting_job, bool mode) {
	// temp code

	set<Job*> children;
	list<Job*> job_list = root_jobs;
	
	if (starting_job) {
		set<Job*>::iterator it;
		children = starting_job->get_children();

		for (it = children.begin(); it != children.end(); ++it) {
			if (((*it)->get_vp_thief() == NULL) && (*it)->compare_and_swap_state(ready, running)) {
				return *it;
			}
		}
	}
	else {
		/* This stretch of code is stupid, I know*/
		
		if (mode) {
			list<Job*>::iterator it;
			for (it = job_list.begin(); it != job_list.end(); ++it) {
				/* We can't return a job that was stolen for another vp*/
				if (((*it)->get_vp_thief() == NULL) && (*it)->compare_and_swap_state(ready, running)) {
					return *it;
				}
				else {
					children = (*it)->get_children();
					if (!children.empty()) {
						job_list.insert(job_list.end(), children.begin(), children.end());
					}
					job_list.remove(*it);
				}
			}
		}
		else {
			list<Job*>::reverse_iterator rit;

			for (rit = job_list.rbegin(); rit != job_list.rend(); ++rit) {
				if (((*rit)->get_vp_thief() == NULL) && (*rit)->compare_and_swap_state(ready, running)) {
					return *rit;
				}
				else {
					children = (*rit)->get_children();
					if (!children.empty()) {
						job_list.insert(job_list.begin(), children.begin(), children.end());
					}
					job_list.remove(*rit);
				}
			}
		}
	}
	return NULL;
}