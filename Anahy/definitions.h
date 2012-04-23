#include <list>
using namespace std;

class Job;

typedef unsigned int                   uint;
typedef unsigned long int              ulong;
typedef void*(*pfunc)(void*); // signature of a parallel funtion
typedef Job*(*sfunc)(list<Job*>, Job*); // signature of a scheduling funtion
typedef void* athread_attr_t; // PROVISORIO!!!