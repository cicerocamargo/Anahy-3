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
	job_map[ job->get_id() ] = job;

	//if (!job->get_parent()) {
		// it is a root job
		root_jobs.push_back(job);
	//}
}

void JobGraph::erase(Job* job) {
	job_map.erase(job->get_id());
	
	if (!job->get_parent()) {
		// it is a root job

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
	if (root_jobs.empty() == false) {
		j = root_jobs.front();
		root_jobs.pop_front();	
	}
	return j; // stub
}