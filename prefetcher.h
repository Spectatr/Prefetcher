/*
 *
 * File: prefetcher.h
 * Author: Sat Garcia (sat@cs)
 * Description: Header file for prefetcher implementation
 *
 */

#ifndef PREFETCHER_H
#define PREFETCHER_H

#include <sys/types.h>
#include "mem-sim.h"
#include <iostream>
#include <queue>
#include <map>
using namespace std;

class GlobalHistory
{
private:
	typedef int DiffAddr;
	typedef int StepsBack;
	typedef u_int64_t Address;

	typedef pair<Address, StepsBack> HPair; 
		
	Address					_lastAddress;
	vector<HPair>			_historyTable;
	map<DiffAddr, int>		_indexTable;

public:
	GlobalHistory() : _lastAddress(0) {}

	void AddMiss(Address address, queue<u_int32_t>& prefetch)
	{
		// If first time, we cannot have a diff
		if (_lastAddress == 0)
		{
			_lastAddress = address;
			return;
		}

		// Compute current diff
		DiffAddr diffAddr = DiffAddr(address - _lastAddress);
		_lastAddress = address;

		StepsBack stepBack = -1;

		// If difference does not exist in IndexTable
		if (_indexTable.count(diffAddr) != 0)
		{
			// If does exist
			int index = _indexTable[diffAddr];
			int itIndex = index;
			stepBack = index;

			// Value of address
			HPair last = _historyTable[index];

			// Limit width
			int limitWidth = _historyTable.size() - 1;
			int limitDepth = 10;

			while (itIndex >= 0 && (limitDepth--) > 0)
			{
				//while (index < limitWidth)
				{
					// Width
					HPair predicted = _historyTable[index + 1];
			
					// Current diff
					DiffAddr currDiff = DiffAddr(predicted.first - last.first);

					// Insert to stack || NEED TO CHECK NO DUPLICATIONS
					prefetch.push(address + currDiff);
					cout << (address + currDiff) << endl;

					last = predicted;
					++index;
				}
				break;

				if (_indexTable.count(itIndex) == 0)
				{
					break;
				}
				itIndex -= _indexTable[itIndex];
				index = itIndex;
			}
		}

		
		HPair hPair(address, stepBack);
		_historyTable.push_back(hPair);
		_indexTable[diffAddr] = _historyTable.size() - 1;
		// Here add implementation for memory limitation
		return;
	}
};


class Prefetcher {
  private:
	queue<u_int32_t> _reqQueue;
	GlobalHistory _globalHistory;

  public:
	Prefetcher();

	// should return true if a request is ready for this cycle
	bool hasRequest(u_int32_t cycle);

	// request a desired address be brought in
	Request getRequest(u_int32_t cycle);

	// this function is called whenever the last prefetcher request was successfully sent to the L2
	void completeRequest(u_int32_t cycle);

	/*
	 * This function is called whenever the CPU references memory.
	 * Note that only the addr, pc, load, issuedAt, and HitL1 should be considered valid data
	 */
	void cpuRequest(Request req); 
};

#endif
