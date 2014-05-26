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

Prefetcher::Prefetcher() :  
	_address_load_diff(0), _address_store_diff(0)
{
	_cFetch = _cReqs = 0;
}

bool Prefetcher::hasRequest(u_int32_t cycle) 
{
	return !_fetchQueue.empty();
}

Request Prefetcher::getRequest(u_int32_t cycle) 
{
	_cFetch = max(_cFetch, long(_fetchQueue.size()));

	Request req = {0};

	req.addr = _fetchQueue.front();

	// Remove handling for this PC
	_fetchQueue.pop();

	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) 
{

}

int sign(long num)
{
	if (num >= 0) return 1;
	return -1;
}

void Prefetcher::cpuRequest(Request req) 
{	
	if (req.load)
	{	
		if (_address_load_diff == 0)
			_address_load_diff = req.addr;

		if (!req.HitL1 || !req.fromCPU)
		{
			_globalHistoryLoads.AddMiss(req.pc, req.addr, _fetchQueue, true);
			int size = _fetchQueue.size();
			for (int i=1; i<=13-size; ++i)
				_fetchQueue.push(req.addr + i*16);

			long last_diff = ((long(req.addr) - long(_address_load_diff)) * 16) / 16;

			if (last_diff > 0 && last_diff < 1024)
				_fetchQueue.push(req.addr + last_diff);

			_address_load_diff = req.addr;
		}
	}
	else
	{
		if (_address_store_diff == 0)
			_address_store_diff = req.addr;

		if (!req.HitL1 || !req.fromCPU)
		{
			_globalHistoryStores.AddMiss(req.pc, req.addr, _fetchQueue, true);
			int size = _fetchQueue.size();
			for (int i=0; i<=13-size; ++i)
				_fetchQueue.push(req.addr + i*16);
		
			long last_diff = ((long(req.addr) - long(_address_store_diff)) * 16) / 16;
			if (last_diff > 0 && last_diff < 1024)
				_fetchQueue.push(req.addr + last_diff);
			
			_address_store_diff = req.addr;
		}
	}

	queue<u_int32_t> tmpQueue;
	set<u_int32_t> tmpSet;

	// Limit size of queue as well
	int limit_queue = 500;

	while (_fetchQueue.size() && limit_queue--)
	{
		u_int32_t curr = _fetchQueue.front();
		_fetchQueue.pop();

		if (tmpSet.count(curr) > 0)
			continue;

		tmpQueue.push(curr);
	}
	
	while (tmpQueue.size())
	{
		_fetchQueue.push(tmpQueue.front());
		tmpQueue.pop();
	}

	return;
}

short GlobalHistory::index1 = 4;
short GlobalHistory::index2 = 1;

int _main()
{
	queue<u_int32_t> fetchThis;
	GlobalHistory _globalHistoryStores;

	// First iteration
	_globalHistoryStores.AddMiss(100, 0, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(200, 1, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(250, 2, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(349, 64, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	// Second iteration
	_globalHistoryStores.AddMiss(200, 65, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(250, 66, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(349, 128, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	//system("pause");

	while(fetchThis.size()) fetchThis.pop();
	_globalHistoryStores.AddMiss(200, 129, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	/*
	// Third iteration
	_globalHistoryStores.AddMiss(5, 129, fetchThis, false);
	_globalHistoryStores.PrintStacks();
	system("pause");

	_globalHistoryStores.AddMiss(20, 130, fetchThis, false);
	_globalHistoryStores.PrintStacks();
	system("pause");

	_globalHistoryStores.AddMiss(100, 192, fetchThis, true);
	_globalHistoryStores.PrintStacks();
	system("pause");
	*/
	while (fetchThis.size())
	{
		cout << "Predicted: " << fetchThis.front() << endl;
		fetchThis.pop();
	}

	_globalHistoryStores.PrintStacks();

	return 0;
}