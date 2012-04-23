#include "Job.h"

ulong Job::counter = 0;

Job::Job() {
    puts("New thread!\n");
    id = counter++;
}

Job::Job (pfunc func, void* job_data) {
    function = func;
    data = job_data;
    id = counter++;
}

void Job::set_parent(Job* job_parent) {
    parent = p;
}

Job* Job::get_parent() const {
    return parent;
}

void set_creator(Job* job_creator) {
    creator = job_creator;
}

Job* Job::get_creator() const {
    return creator;
}

ThreadState Job::get_state() const {
    return state;
}

ulong Job::get_id() const {
    return id;
}

void run() {
    (function)(data);
}