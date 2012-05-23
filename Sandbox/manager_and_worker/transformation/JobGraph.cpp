#include "JobGraph.h"
#include "JobId.h"
#include "Job.h"

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
	job_map.erase(job->get_id());
	
	if (!job->get_parent()) {
		list<Job*>::iterator it;
		for (it = root_jobs.begin(); it != root_jobs.end(); ++it) {
			if (*it == job) {
				it = root_jobs.erase(it);
				delete job;
				break;
			}	
		}
	}	
}

Job* JobGraph::find_a_ready_job(Job* starting_job) {
	Job* j = NULL;
	if (!root_jobs.empty()) {
		j = root_jobs.front();
		root_jobs.pop_front();	
	}
	if (!job_map.empty()) {
		map<JobId, Job*>::iterator it;
		it = job_map.begin();
		j = (*it).second;
		job_map.erase(j->get_id());
	}
	return j; // stub
}