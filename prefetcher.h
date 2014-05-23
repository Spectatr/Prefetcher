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
#include <set>
#include <list>
#include <climits>

using namespace std;


class HistoryBuffer {
private:
	typedef long Index;
	typedef u_int32_t PC;
	typedef long PC_Pointer;
	typedef u_int32_t Addr;
	typedef long Addr_Pointer;
	typedef long Stride;
	enum State { Init, Steady, Transient, NoPred };

	typedef pair<Addr, Addr_Pointer> MyHistory;
	typedef pair<PC, Index> MyIndex;

	list<MyIndex> _indexTable;
	vector<MyHistory> _historyBuffer;

public:
	long _cIndex;
	long _cBuff;

public:
	HistoryBuffer() : _cIndex(0), _cBuff(0) {}

	template<class T1, class T2>
	T2 doesExist(list<pair<T1, T2>>& myList, T1& obj)
	{
		for (list<pair<T1, T2>>::iterator it = myList.begin();
			it != myList.end();
			++it)
		{
			if (it->first == obj) return it->second;
		}
		return 0;
	}

	// http://www.cs.iit.edu/~chen/docs/chen_sc07-dahc.pdf
	// adress = real address for global, pc for local (I think)
	void AddMiss(Addr address, PC pc, vector<u_int32_t>& prefetch)
	{
		Index idx = 0;

		if (doesExist<PC, Index>(_indexTable, address))
		{
			MyHistory myHist(address, Addr_Pointer(LONG_MAX));
			_historyBuffer.push_back(myHist);

			_cBuff = max(_cBuff, long(_historyBuffer.size()));

			idx = _historyBuffer.size() - 1;
			_indexTable.push_back(MyIndex(address, idx));
			if (_indexTable.size() > 64)
			{
				_indexTable.pop_front();
			}

			_cIndex = max(_cIndex, long(_indexTable.size()));

			//for (int i = 1; i < 12; ++i)
			//	prefetch.push_back(address + 16*i);

			return;
		}


		long currLast = _historyBuffer.size() - 1;
		long lastIdx = idx = doesExist<PC, Index>(_indexTable, address);
		MyHistory& hist = _historyBuffer[idx];

		while (idx < currLast)
		{
			for (int j = 1; j <= 6; ++j)
			{
				if (idx + j >= lastIdx)
				{
					break;
				}
				MyHistory next = _historyBuffer[idx + j];

				Addr nextAddr = next.first;

				if (nextAddr != address)
				{
					prefetch.push_back(next.first);
				}
			}

			idx = hist.second;

			if (idx == LONG_MAX)
				break;

			hist = _historyBuffer[idx];
		}

		MyHistory myHist(address, Addr_Pointer(lastIdx));
		_historyBuffer.push_back(myHist);

		_cBuff = max(_cBuff, long(_historyBuffer.size()));

		idx = _historyBuffer.size() - 1;
		// over the limit because of h=ordere hehrherhe
		_indexTable.push_back(MyIndex(address, idx));
		if (_indexTable.size() > 64)
		{
			_indexTable.pop_front();
		}

		_cIndex = max(_cIndex, long(_indexTable.size()));
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
	HistoryBuffer _globalHistoryBufferStore;
	
	queue<u_int32_t> _fetchQueue;
	set<u_int32_t> _reqsMap;

	long _cFetch;
	long _cReqs;

	u_int32_t last_address;
	u_int32_t last_address_store;
	long last_diff;
	long last_diff_store;

public:
	Prefetcher();

	~Prefetcher()
	{
		cout << "_globalHistoryBuffer: " << _globalHistoryBuffer._cIndex * 8 / 1024.0 << ", "
			<< _globalHistoryBuffer._cBuff * 8 / 1024.0 << endl;
		cout << "_globalHistoryBufferStore: " << _globalHistoryBufferStore._cIndex * 8 / 1024.0 << ", "
			<< _globalHistoryBufferStore._cBuff * 8 / 1024.0 << endl;
		cout << "_cFetch: " << _cFetch * 4 / 1024.0 << endl;
		cout << "_cReqs: " << _cReqs * 4 / 1024.0 << endl;
	}

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
