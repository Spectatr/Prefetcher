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

void Prefetcher::cpuRequest(Request req)
{


	/*
	if (!req.HitL1) 
	{
		u_int32_t pc = req.pc >> 4;
		long offs = 16;
		long n = 12;

		if (_offsMap.find(pc) != _offsMap.end())
		{
			offs = _offsMap[pc];		
		}
		else if (_lastMap.find(pc) != _lastMap.end())
		{
			_offsMap[pc] = (offs = (req.addr - _lastMap[address]));
		}
		
		if (offs < 0)
		{
			offs = 16;
			_nameSets[address] = 0;
		}

		_lastMap[address] = req.addr;
 		n += ++_nameSets[address];

		for (int i=0; i<n; ++i)
		{
			Request tmp_req;
			tmp_req.addr = req.addr + offs * (i+1);

			if (_reqsMap[tmp_req])
				break;
		
			_reqsQueue.push_back(tmp_req);
			_reqsMap[tmp_req] = true;
		}
	}


	/* 1.615163 * /
	if (!req.HitL1) {
		static u_int32_t last_address = 0;
		static long last_diff = 16;

		if (last_address)
		{
			last_diff = req.addr - last_address;
			if (abs(last_diff) > 16*2)
				last_diff = 16 * (last_diff > 0 ? 1 : -1);
		}
		last_address = req.addr;
	

		for (int i=0; i<N; ++i)
		{
			Request tmp_req;
			tmp_req.addr = req.addr + last_diff * (i+1);

			if (_reqsMap[tmp_req])
				break;
		
			_reqsQueue.push_back(tmp_req);
			_reqsMap[tmp_req] = true;
		}
	}*/
}
