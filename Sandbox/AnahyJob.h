#include <cassert>

typedef void*(*ParFunc)(void*);

class AnahyJob {
	unsigned long _id;
	unsigned short _join_counter;
	unsigned short _fork_counter;
	bool _smart;
	ParFunc _function;
	void* _args;
	void* _result;
	AnahyJobAttributes* _attr;

	// hiding constructors
	AnahyJob() {}
	AnahyJob(AnahyJob&) {}
	
	// private methods that can be called by a VP
	void set_id();
	void run();

public:

	AnahyJob(ParFunc function, void* args, AnahyJobAttributes* attr, bool smart=false);
	~AnahyJob();
	static AnahyJob* new_smart_job(ParFunc function, void* args, AnahyJobAttributes* attr);
	
	unsigned short join_counter() { return _join_counter; }
	void set_join_counter(unsigned short join_counter);
	
	unsigned short fork_counter() { return _fork_counter; }
	void set_fork_counter(unsigned short fork_counter);
	
	void atomic_decrement_join_counter() { __sync_fetch_and_sub(&_join_counter, 1); }
	void atomic_decrement_fork_counter() { __sync_fetch_and_sub(&_fork_counter, 1); }
	
	unsigned short id() { return _id; }
	bool smart() { return _smart; }
	
	AnahyJobAttributes* attributes() { return _attr; }
	void set_attributes(AnahyJobAttributes* attr) { _attr = attr; }
};