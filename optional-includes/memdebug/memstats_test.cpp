/*
 *  memstats_test.cpp
 *  OpenLieroX
 *
 *  This is some testing code for memstats.h.
 *
 *  Created by Albert Zeyer on 03.09.09.
 *  code under LGPL
 *
 */

#include <iostream>
using namespace std;

struct S1 { int a, b; };
struct S2 { int a, b; };

int main() {
	cout << "-- start" << endl;
	
	S1* s1 = new S1();
	S2* s2 = new S2[10];
	
	cout << "-- middle" << endl;
	
	delete s1;
	delete[] s2;
	
	cout << "-- end" << endl;
}
