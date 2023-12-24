#include <iostream>

using namespace std;

void dbg(bool b){
	if(b){
		cout << "dbg" << endl;
	}
}

bool inside(int min, int v, int max){
	if(min <= v && v <= max) return true;
	return false;
}

bool contact(int amin, int amax, int bmin, int bmax){
	if(inside(amin, bmin, amax) || inside(amin, bmax, amax)){
		return true;
	}
	return false;
}