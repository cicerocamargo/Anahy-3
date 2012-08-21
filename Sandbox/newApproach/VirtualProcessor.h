#include <pthread.h>
#include <stack>
#include "definitions.h"

using namespace std;

class Daemon;
class Job;
class JobHandle;
class JobAttributes;

class VirtualProcessor {
	// a unique id for this VP
	uint id;

	Daemon* daemon;

	//to set the vp affinity
	int tid;
	
	//tracks how many VP objects have been created
	static uint instance_counter;
	static long tid_counter;
	// this is the pointer to the graph of this vp
		
	// the job that is running at the moment
	Job* current_job;
	
	// the number of jobs created by this VP
	ulong job_counter;

	/* a key to store an VP object
	 * in and retrieve it from a pthread
	 * is set in 'call_vp_run' and
	 * got in 'static get_current_vp' from API
	 */
	static pthread_key_t key;
	
	//to keep track of jobs blocked on this VP
	stack<Job*> context_stack;
	
	pthread_t thread; // my thread
	static pthread_mutex_t mutex; // where I wait for daemon answers

	/* called from 'this->thread' to set thread
	 * specific data as this and call this->run() (vp_obj is 'this')
	*/
	static void* call_vp_run(void* vp_obj);

	void suspend_current_job_and_try_to_help(Job* joined);
	void suspend_current_job_and_run_another(Job* another);

public:

	VirtualProcessor(Daemon* _daemon);
	~VirtualProcessor();

	void run(); // called from call_vp_run (begins the VP loop)

	/* two class methods to initialize and destroy the pthread_key
	 * they should be called once, before any VP is created and after any VP is destroyed
	 * so they are called from AnahyVM::boot and AnahyVM::shut_down
	 */
	static void init_pthread_key();
	static void delete_pthread_key();
	
	//just to fill pthread_key_create requirements
	static void call_vp_destructor(void *vp_obj);

	static void associate_vp_with_current_thread(void* vp_obj);
	
	// messages to be received from Daemon
	static VirtualProcessor* get_current_vp();
	JobHandle create_new_job(pfunc function, void* args, JobAttributes* attr);

	//this will be complicated to do
	void* join_job(JobHandle handle);

	// messages to be received from a Daemon
	void start();
	void stop();

	/* getters and setters */
	inline Job* get_current_job() const { return current_job; }
	inline void set_current_job(Job* new_value) { current_job = new_value; }
	inline long get_tid() { return tid; }
	inline void set_tid(long _tid) { tid = _tid; }
	uint get_id() const;
	ulong get_job_counter() const;
};