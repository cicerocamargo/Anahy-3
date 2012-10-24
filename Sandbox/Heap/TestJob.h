#include <iostream>

class TestJob
{
	static int counter;
	int _id;
public:
	TestJob() {
		_id = counter++;
	}

    bool operator<=(TestJob& j) {
        return _id < j._id;
    }
        
    bool operator>=(TestJob& j) {
        return _id > j._id;
    }
    
};

int TestJob::counter = 0;