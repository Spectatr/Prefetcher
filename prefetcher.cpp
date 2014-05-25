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

Prefetcher::Prefetcher() 
{  
}

bool Prefetcher::hasRequest(u_int32_t cycle) 
{ 
	return !_reqQueue.empty(); 
}

Request Prefetcher::getRequest(u_int32_t cycle) 
{ 
	Request retRequest = {0};
	retRequest.addr = _reqQueue.front();
	_reqQueue.pop();

	return retRequest;
}

void Prefetcher::completeRequest(u_int32_t cycle) { }

void Prefetcher::cpuRequest(Request req) 
{ 
	if(!req.HitL1 && req.load) {
		_globalHistory.AddMiss(req.addr, _reqQueue);
		//cout << _reqQueue.size() << endl;
	}
}
