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
#include <cmath>

Prefetcher::Prefetcher() : last_address(INT_MIN), last_diff(16), last_address_store(INT_MIN), last_diff_store(16) { 
	_cFetch = _cReqs = 0;
}

bool Prefetcher::hasRequest(u_int32_t cycle) 
{
	return !_fetchQueue.empty();
}

Request Prefetcher::getRequest(u_int32_t cycle) 
{
	_cFetch = max(_cFetch, long(_fetchQueue.size()));
	_cReqs = max(_cReqs, long(_reqsMap.size()));

	Request req = {0};

	req.addr = _fetchQueue.front();
	_reqsMap.erase(req.addr);

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
	if (req.load)
	{
		_globalHistoryBuffer.AddMiss(req.addr, req.pc, fetchThis);

		if (!req.HitL1) 
		{
			for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
				it != fetchThis.end(); 
				++it)
			{
				_fetchQueue.push(*it);
			}
			
			if (last_address)
			{
				last_diff = req.addr - last_address;
				if (abs(last_diff) > 16 * 2)
					last_diff = 16 * (last_diff > 0 ? 1 : 1);
			}

			last_address = req.addr;


			for (int i = 0; i < 10; ++i)
			{
				Request tmp_req;
				tmp_req.addr = req.addr + last_diff * (i + 1);

				if (_reqsMap.count(tmp_req.addr))
					break;

				_fetchQueue.push(tmp_req.addr);
				_reqsMap.insert(tmp_req.addr);
			
			}
		}
	}
	else
	{
		_globalHistoryBufferStore.AddMiss(req.addr, req.pc, fetchThis);
		if (!req.HitL1) 
		{
			for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
				it != fetchThis.end(); 
				++it)
			{
				_fetchQueue.push(*it);
			}
	
			
			if (last_address_store)
			{
				last_diff_store = req.addr - last_address_store;
				if (abs(last_diff_store) > 16 * 2)
					last_diff_store = 16 * (last_diff_store > 0 ? 1 : -2);
			}

			last_address_store = req.addr;

			for (int i = 0; i < 13; ++i)
			{
				Request tmp_req;
				tmp_req.addr = req.addr + last_diff_store * (i + 1);

				if (_reqsMap.count(tmp_req.addr))
					break;

				_fetchQueue.push(tmp_req.addr);
				_reqsMap.insert(tmp_req.addr);
			}
		}
	}
}
