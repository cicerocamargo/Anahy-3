#ifndef JOB_H
#define JOB_H

#include "definitions.h"
#include <pthread.h>

class VirtualProcessor;

enum JobState {ready, running, finished, blocked};

class Job {
    
        static ulong counter;
        ulong id;
        pfunc function;
        void* data;
        void* retval;
  
        Job* parent;
        set<Job*> children;
        VirturalProcessor* vp_creator;
        JobState state;
        athread_attr_t job_attributes;
        
public:
        Job();
        Job (pfunc func, void* job_data);
        //virtual ~athread_t ();
        void run();
        void add_child(Job* child);
        // getters and setters
        void set_parent(Job* job_parent);
        
        Job* get_parent() const;
        VirturalProcessor* get_vp_creator() const;
        JobState get_state() const;
        ulong get_id() const;
        
};

#endif