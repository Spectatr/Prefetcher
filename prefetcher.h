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
#include <list>

using namespace std;

class HistoryBuffer {
private:
	typedef long Index;
	typedef u_int32_t PC;
	typedef long PC_Pointer;
	typedef u_int32_t Addr;
	typedef long Addr_Pointer;
	typedef long Stride;
	enum State {Init, Steady, Transient, NoPred};

	typedef tuple<Addr, Addr_Pointer> MyHistory;

	map<PC, Index> _indexTable;
	map<Addr, Index> _pcTable;

	vector<MyHistory> _historyBuffer;

public:
	// http://www.cs.iit.edu/~chen/docs/chen_sc07-dahc.pdf
	// adress = real address for global, pc for local (I think)
	void AddMiss(Addr address, PC pc, vector<u_int32_t>& prefetch)
	{
		Index idx = 0;

		if (_indexTable.find(address) == _indexTable.end())
		{
			MyHistory myHist(address, Addr_Pointer(LONG_MAX));
			_historyBuffer.push_back(myHist);

			idx = _historyBuffer.size() - 1;
			_indexTable[address] = idx;

			//for (int i = 1; i < 12; ++i)
				//prefetch.push_back(address + 16*i);
		
			return;
		}		 
		
		
		long currLast = _historyBuffer.size() - 1;
		long lastIdx = idx = _indexTable[address];
		MyHistory& hist = _historyBuffer[idx];

		while (idx < currLast)
		{
			for (int j = 1; j <=6; ++j)
			{
				if (idx + j >= lastIdx)
				{
					break;
				}
				MyHistory& next = _historyBuffer[idx+j];
		
				Addr nextAddr = get<0>(next);
			
				if (nextAddr != address)
				{
					prefetch.push_back(get<0>(next));
				}
			}

			idx = get<1>(hist);
			
			if (idx == LONG_MAX)
				break;

			hist = _historyBuffer[idx];
		}

		MyHistory myHist(address, Addr_Pointer(lastIdx));
		_historyBuffer.push_back(myHist);

		idx = _historyBuffer.size() - 1;
		_indexTable[address] = idx;
		
	}
};

class my_less
{
public:
	bool operator()(Request lhd, Request rhs)
	{
		return lhd.addr > rhs.addr;
	}
};


class Prefetcher {
  private:
	HistoryBuffer _globalHistoryBuffer;
	queue<tuple<u_int32_t, u_int32_t>> _fetchQueue;

	map<Request, bool, my_less> _reqsMap;
	map<u_int32_t, long> _offsMap;
	map<u_int32_t, u_int32_t> _lastMap;
	map<u_int32_t, long> _nameSets;
	list<Request> _reqsQueue;

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
