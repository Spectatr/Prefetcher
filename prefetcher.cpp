/*
*
* File: prefetcher.C
* Author: Sat Garcia (sat@cs)
* Description: This simple prefetcher waits until there is a D-cache miss then
* requests location (addr + 16), where addr is the address that just missed
* in the cache.
*
*/

#include "prefetcher.h"
#include <stdio.h>

Prefetcher::Prefetcher() : last_address(0), last_diff(16) { }

bool Prefetcher::hasRequest(u_int32_t cycle) 
{
	return !_fetchQueue.empty();
}

Request Prefetcher::getRequest(u_int32_t cycle) 
{
	Request req = {0};

	auto reqPair = _fetchQueue.front();

	req.addr = get<0>(reqPair);
	_reqsMap[req] = false;

	// Remove handling for this PC
	_fetchQueue.pop();

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) 
{

}

void Prefetcher::cpuRequest(Request req) 
{	
	vector<u_int32_t> fetchThis;
	_globalHistoryBuffer.AddMiss(req.addr, req.pc, fetchThis);

	if (!req.HitL1) {
		
		for (auto it = fetchThis.begin(); it != fetchThis.end(); ++it)
		{
			_fetchQueue.push(make_pair(*it, req.addr));
		
		}
		
		if (last_address)
		{
			last_diff = req.addr - last_address;
			if (abs(last_diff) > 16 * 2)
				last_diff = 16 * (last_diff > 0 ? 1 : 1);
		}

		last_address = req.addr;


		for (int i = 0; i < 13; ++i)
		{
			Request tmp_req;
			tmp_req.addr = req.addr + last_diff * (i + 1);

			if (_reqsMap[tmp_req])
				break;

			_fetchQueue.push(make_pair(tmp_req.addr, tmp_req.addr));
			_reqsMap[tmp_req] = true;
			
		}
	}
}
