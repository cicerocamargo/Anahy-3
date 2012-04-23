#ifndef JOB_H
#define JOB_H

#include "definitions.h"

enum ThreadState {none, ready, running, finished, blocked};

class Job {
    
        static ulong counter;
        ulong id;
        pfunc function;
        void* data;
        void* retval;
  
        Job* parent;
        Job* creator;
        ThreadState state;
        athread_attr_t job_attributes;
        
public:
        Job();
        Job (pfunc func, void* job_data);
        //virtual ~athread_t ();

        // getters and setters
        void set_parent(Job* job_parent);
        Job* get_parent() const;
        void set_creator(Job* job_creator);
        Job* get_creator() const;
        ThreadState get_state() const;
        ulong get_id() const;
        void run();
};

#endif