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
	return _prefech.size();
}

Request Prefetcher::getRequest(u_int32_t cycle) { 
	Request req;
	req.addr = _prefech.front();
	_prefech.pop();

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { 
	// ????
}

void Prefetcher::cpuRequest(Request req) { 
	//if(!req.HitL1 && !req.HitL2);
	
	_prefetchLast.AddAddress(req.addr);

	if (!req.HitL1 && _prefetchLast.IsReady())
	{
		_prefetchLast.PrefetchSome(_prefech);
	}
}
