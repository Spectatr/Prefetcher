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
#include <map>
#include <vector>
#include <queue>
#include <tuple>
#include <set>

using namespace std;



class HistoryBuffer {
private:
	typedef tuple<u_int32_t, long> MyHistory;

	map<u_int32_t, long> _indexTable;
	map<u_int32_t, long> _pcTable;

	vector<MyHistory> _historyBuffer;

public:
	// http://www.cs.iit.edu/~chen/docs/chen_sc07-dahc.pdf
	// adress = real address for global, pc for local (I think)
	void AddMiss(u_int32_t address, u_int32_t pc, vector<u_int32_t>& prefetch)
	{
		//cout << "Address is " << address << endl;;
		long ptr = 0;

		// Have never seen this address before
		if (_indexTable.find(address) == _indexTable.end())
		{
			if (true /* why the hybrid takes longer?! */ || _pcTable.find(pc) == _pcTable.end())
			{
				MyHistory newHistory(std::make_tuple(address, 0));
				_historyBuffer.push_back(newHistory);

				_indexTable[address] = _historyBuffer.size() - 1;
				_pcTable[address] = _historyBuffer.size() - 1;

				for (int i=1; i<12; ++i)
				{
					prefetch.push_back(address + 16*i);
					MyHistory newHistory(std::make_tuple(address + 16*i, 0));
					_historyBuffer.push_back(newHistory);
				}
				return;
			}
			ptr = _pcTable[pc];
		}
		else 
		{
			ptr = _indexTable[address];
		}

		//system("pause");

		// Else - there must be below a newer (other) address
		// otherwise it could not be a miss!!!
		long firstPtr = ptr;

		if (ptr == _historyBuffer.size() - 1)
		{
			// Temporary
			//goto MyLabel;

			return;
		}

		while (ptr)
		{
			prefetch.push_back(std::get<0>(_historyBuffer[ptr + 1]));
			ptr = std::get<1>(_historyBuffer[ptr]);
		}

		// Just add the new memory
		MyHistory newHistory(std::make_tuple(address, firstPtr));
		_historyBuffer.push_back(newHistory);
		_indexTable[address] = _historyBuffer.size() - 1;
		_pcTable[pc] = _historyBuffer.size() - 1;
		return;
	}
};


class Prefetcher {
  private:
	HistoryBuffer _globalHistoryBuffer;
	queue<tuple<u_int32_t, u_int32_t>> _fetchQueue;

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
