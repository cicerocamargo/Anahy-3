#include <list>
using namespace std;

class Job;

typedef unsigned int                   uint;
typedef unsigned long int              athread_t;
typedef void*(*pfunc)(void*); // signature of a parallel funtion
typedef Job*(*sfunc)(list<Job*>, Job*); // signature of a scheduling funtion
