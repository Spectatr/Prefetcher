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

Prefetcher::Prefetcher() {}

bool Prefetcher::hasRequest(u_int32_t cycle) { 
	return !_fetchQueue.empty();
}

Request Prefetcher::getRequest(u_int32_t cycle) { 
	Request req = {0};
	
	auto reqPair = _fetchQueue.front();

	// Remove handling for this PC
	req.addr = get<0>(reqPair);

	_fetchQueue.pop();

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { 
	// ????
}

void Prefetcher::cpuRequest(Request req) { 
	if(!req.HitL1 && !req.HitL2) {

		//cout << "Address " << req.addr << endl;
		//system("pause");
		vector<u_int32_t> fetchThis;
		_globalHistoryBuffer.AddMiss(req.addr, req.pc, fetchThis);

		for (auto it = fetchThis.begin(); it != fetchThis.end(); ++it)
		{
			_fetchQueue.push(make_pair(*it, req.addr));
		}
	}
}
