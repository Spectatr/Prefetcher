#include "prefetcher.h"
#include "mem-sim.h"
#include <iostream>
#include <assert.h>
using namespace std;

bool Prefetcher::hasRequest(u_int32_t cycle) {
	return _q.size();
}

Request Prefetcher::getRequest(u_int32_t cycle) {
	//cout << "Index is: " << _i << " and cycle is " << cycle << endl;
	//Request req = _reqsQueue.front();
	my_pair tup = _q.top();
	Request req = std::get<1>(tup);
	//cout << std::get<0>(tup) << " - ";

	_q.pop();
	_reqsMap[req] = false;

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { 
	return; 
}

void Prefetcher::cpuRequest(Request req) {
	if (!req.HitL1) {
		int x = 0;
		for (int i=0; i<N+x; ++i)
		{
			Request tmp_req;
			tmp_req.addr = req.addr + 16 * (i+1);
			//cout << tmp_req.addr << endl;

			if (_reqsMap[tmp_req] == true)
			{
				++x;
				continue;
			}
		
			_q.push(my_pair(i+x, tmp_req));
			_reqsMap[tmp_req] = true;
		}
	}
}
