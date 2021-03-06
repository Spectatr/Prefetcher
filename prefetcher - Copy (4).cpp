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
	static u_int32_t last_cycle = cycle;
	static int count = 0;

	if (_fetchQueue.empty())
	{
		last_cycle = cycle;
		count = 10;
		return false;
	}

	// else
	if (cycle - last_cycle >= 46)
	{
		count = 10;
		last_cycle = cycle;
		return true;
	}

	if (count > 0)
	{
		--count;
		return true;
	}

	return false;
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

/*
double min(double a, double b)
{
	return a < b ? a : b;
}

double max(double a, double b)
{
	return a > b ? a : b;
}

long round(double a)
{
	if ((a - long(a)) > 0.5)
	{
		return (long(a) + 1);
	}
	return long(a);
}
*/

void Prefetcher::cpuRequest(Request req) 
{	
	vector<u_int32_t> fetchThis;
	if (req.load)
	{
		static u_int32_t llast_address = 0;
		
		if (llast_address)
			_globalHistoryBuffer.AddMiss(req.addr - llast_address, fetchThis);

		llast_address = req.addr;

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
		static u_int32_t llast_address = 0;

		if (llast_address)
			_globalHistoryBufferStore.AddMiss(req.addr - llast_address, fetchThis);

		llast_address = req.addr;

		//_globalHistoryBufferStore.AddMiss(req.addr, req.pc, fetchThis);
		if (!req.HitL1) 
		{
			for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
				it != fetchThis.end(); 
				++it)
			{
				cout << "Store: " << *it << endl;
				_fetchQueue.push(*it);
			}
			if (fetchThis.size())
				cout << "================" << endl;
	
			
			if (last_address_store)
			{
				last_diff_store = req.addr - last_address_store;
				if (abs(last_diff_store) > 16 * 2)
				{
				//	cout << "Diff is " << last_diff_store << endl;
					last_diff_store = 16 * (last_diff_store > 0 ? 1 : -2);
				}
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
