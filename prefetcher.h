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

			for (int i = 1; i < 12; ++i)
				prefetch.push_back(address + 16*i);
		
			return;
		}		 
		
		long lastIdx = idx = _indexTable[address];
		MyHistory& hist = _historyBuffer[idx];

		idx = get<1>(hist);
		int count = 0;

		while (idx != LONG_MAX && count++ < 5)
		{
			MyHistory& hist = _historyBuffer[idx];
	
			for (int i = 1; idx + i < lastIdx && i < 4; ++i)
			{
				prefetch.push_back(get<0>(_historyBuffer[idx+i]));
			}
			
			idx = get<1>(hist);
		}

		MyHistory myHist(address, Addr_Pointer(lastIdx));
		_historyBuffer.push_back(myHist);

		idx = _historyBuffer.size() - 1;
		_indexTable[address] = idx;
		
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
