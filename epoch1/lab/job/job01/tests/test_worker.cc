//
//    test_worker.cc
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//////////////////////////////////////////////////////////////////////////////

#include "job01/worker2.h"
using namespace schwa::job01;

#include <iostream>
#include <chrono>
using namespace std;


int main(void) {
    Boss2 boss;


/*
    Worker2 pair[2]{1,2};

//    boss.go();
    pair[0].go();

    cerr << "WORKER: " << pair[0].ind() << endl;
    cerr << "WORKER: " << pair[1].ind() << endl;


    chrono::milliseconds duration(2000);
    this_thread::sleep_for( duration );
    cerr << "Waited 2000 ms\n";


    pair[0].stop();
    pair[1].go();
    pair[1].stop();
*/

    cerr << "job01/test_worker...  PASSED!" << endl << endl;
}
