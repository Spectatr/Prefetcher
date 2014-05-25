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
	_last_address_load(0), _last_address_store(0), 
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

int sign(long num)
{
	if (num >= 0) return 1;
	return -1;
}

void Prefetcher::cpuRequest(Request req) 
{	
	vector<u_int32_t> fetchThis;

	if (req.load)
	{
		if (_last_address_load == 0)
		{
			_last_address_load = req.addr;
			return;
		}

		long currDiff = ((req.addr - _last_address_load) / 16) * 16;

		if (!req.HitL1) 
		{
			_globalHistoryLoads.AddMiss(req.addr, req.pc, fetchThis, !req.HitL1);

			for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
				it != fetchThis.end(); 
				++it)
			{
				_fetchQueue.push(*it);
			}
			

			if (false && currDiff && currDiff == _address_load_diff)
			{
				for (int i = 0; i < 10; ++i)
				{
					Request tmp_req;
					tmp_req.addr = req.addr + currDiff * i;		// (maybe i+1???)

					if (_reqsMap.count(tmp_req.addr))
						continue;

					_fetchQueue.push(tmp_req.addr);
					_reqsMap.insert(tmp_req.addr);
				}
			}
			
			for (int i = 1; i <= 0; ++i)
			{
				Request tmp_req;
				tmp_req.addr = req.addr + 16 * i;		// (maybe i+1???)

				if (_reqsMap.count(tmp_req.addr))
					continue;

				_fetchQueue.push(tmp_req.addr);
				_reqsMap.insert(tmp_req.addr);
			}
		}

		_address_load_diff = currDiff;
		_last_address_load = req.addr;
	}
	else
	{
		if (_last_address_store == 0)
		{
			_last_address_store = req.addr;
			return;
		}

		long currDiff = ((req.addr - _last_address_store) / 16) * 16;

		if (!req.HitL1) 
		{
			_globalHistoryStores.AddMiss(req.addr, req.pc, fetchThis, !req.HitL1);

			for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
				it != fetchThis.end(); 
				++it)
			{
				_fetchQueue.push(*it);
			}

			if (!req.HitL1) 
			{
				for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
					it != fetchThis.end(); 
					++it)
				{
					_fetchQueue.push(*it);
				}
			

				if (false && currDiff == _address_store_diff)
				{
					for (int i = 0; i < 10; ++i)
					{
						Request tmp_req;
						tmp_req.addr = req.addr + currDiff * i;		// (maybe i+1???)

						if (_reqsMap.count(tmp_req.addr))
							continue;

						_fetchQueue.push(tmp_req.addr);
						_reqsMap.insert(tmp_req.addr);
					}
				}

				for (int i = 1; i <= 0; ++i)
				{
					Request tmp_req;
					tmp_req.addr = req.addr + 16 * i * sign(currDiff);		// (maybe i+1???)

					if (_reqsMap.count(tmp_req.addr))
						continue;

					_fetchQueue.push(tmp_req.addr);
					_reqsMap.insert(tmp_req.addr);
				}
			}
		}
		
		_address_store_diff = currDiff;
		_last_address_store = req.addr;
	}
}



int _main()
{
	vector<u_int32_t> fetchThis;
	GlobalHistory _globalHistoryStores;

	// First iteration
	_globalHistoryStores.AddMiss(0, 0, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(1, 1, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(2, 2, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(64, 64, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	// Second iteration
	_globalHistoryStores.AddMiss(65, 65, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(66, 66, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	_globalHistoryStores.AddMiss(128, 128, fetchThis, true);
	_globalHistoryStores.printStacks();
	//system("pause");

	fetchThis.clear();
	_globalHistoryStores.AddMiss(129, 129, fetchThis, true);
	_globalHistoryStores.printStacks();
	/*
	// Third iteration
	_globalHistoryStores.AddMiss(5, 129, fetchThis, false);
	_globalHistoryStores.printStacks();
	system("pause");

	_globalHistoryStores.AddMiss(20, 130, fetchThis, false);
	_globalHistoryStores.printStacks();
	system("pause");

	_globalHistoryStores.AddMiss(100, 192, fetchThis, true);
	_globalHistoryStores.printStacks();
	system("pause");
	*/
	for (vector<u_int32_t>::iterator it = fetchThis.begin(); 
		it != fetchThis.end(); 
		++it)
	{
		cout << "Predicted: " << *it << endl;
	}


	return 0;
}