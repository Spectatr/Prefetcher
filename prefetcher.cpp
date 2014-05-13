#include "prefetcher.h"
#include "mem-sim.h"
#include <iostream>
using namespace std;

bool Prefetcher::hasRequest(u_int32_t cycle) {
	return _reqsQueue.size();
}

Request Prefetcher::getRequest(u_int32_t cycle) {
	//cout << "Index is: " << _i << " and cycle is " << cycle << endl;
	Request req = _reqsQueue.front();
	_reqsQueue.pop_front();
	_reqsMap[req] = false;

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { 
	return; 
}

void Prefetcher::cpuRequest(Request req) {
	if (!req.HitL1 || !req.HitL2) {
		if (_reqsQueue.size() > 25)
		{
			_reqsQueue.clear();
			_reqsMap.clear();
		}
		for (int i=0; i<N; ++i)
		{
			Request tmp_req;
			tmp_req.addr = req.addr + 16 * (i+1);

			if (_reqsMap[tmp_req])
				break;
		
			_reqsQueue.push_back(tmp_req);
			_reqsMap[tmp_req] = true;
		}
	}
}
