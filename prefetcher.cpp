#include "prefetcher.h"
#include "mem-sim.h"
#include <iostream>
using namespace std;

bool Prefetcher::hasRequest(u_int32_t cycle) {
	if (_i >= _N) _ready = false;
	return _ready;
}

Request Prefetcher::getRequest(u_int32_t cycle) {
	//cout << "Index is: " << _i << " and cycle is " << cycle << endl;
	Request req = _nextReq[_i++];
	_ready = (_i < _N);
	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { 
	return; 
}

void Prefetcher::cpuRequest(Request req) {
	if (!_ready && !req.HitL1) {
		for (int i=0; i<_N; ++i)
		{
			_nextReq[i].addr = req.addr + 16 * (i+1);
		}
		_i = 0;
		_ready = true;
	}
}
