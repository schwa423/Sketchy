#include "job01/jobqueue.h"

using namespace schwa;
using namespace schwa::job01;

#include <iostream>
using std::cerr;
using std::endl;

int main() {

//	JobPool pool;
	JobQueueRing<4> ring;

	JobQueue& q0 = *ring.elementAt(0);
	JobQueue& q1 = *ring.elementAt(1);
	JobQueue& q2 = *ring.elementAt(2);
	JobQueue& q3 = *ring.elementAt(3);







    cerr << "job01/test_jobqueue...  NOTHING TO TEST!" << endl << endl;
}

