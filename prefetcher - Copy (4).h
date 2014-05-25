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

#define BUFFER_SIZE 128
#define INDEX_SIZE 128

class HistoryBuffer {
private:
	typedef long Index;
	//typedef u_int32_t PC;
	//typedef long PC_Pointer;
	//typedef u_int32_t Addr;
	typedef int Diff;
	//typedef long Addr_Pointer;
	//typedef long Stride;
	//enum State { Init, Steady, Transient, NoPred };

	typedef pair<Diff, Index> MyHistory;
	typedef pair<Diff, Index> MyIndex;

	list<MyIndex> _indexTable;
	MyHistory _historyBuffer[BUFFER_SIZE];

	long _buffSize;
	long _posBuffer;	// can be int
	long _limBuffer;


public:
	long _cIndex;
	long _cBuff;

public:
	HistoryBuffer() : _cIndex(0), _cBuff(0), _posBuffer(-1), _limBuffer(0), _buffSize(0)
	{
		for (int i=0; i<BUFFER_SIZE; ++i)
		{
			_historyBuffer[i] = MyHistory(0, -1);
		}
	}

	template<class T>
	Index exists(list<MyIndex>& myList, T& obj)
	{
		typename list<MyIndex>::iterator it;
		for (it = myList.begin();
			it != myList.end();
			++it)
		{
			if (it->first == obj) return it->second;
		}
		return -1;
	}

	template<class T>
	Index addObject(T& obj)
	{
		//cout << _posBuffer << endl;
		_posBuffer = ((_posBuffer + 1) % BUFFER_SIZE);
		_historyBuffer[_posBuffer] = obj;

		if (++_buffSize >= BUFFER_SIZE)
		{
			++_limBuffer;
			--_buffSize;
		}

		return (_posBuffer % BUFFER_SIZE);
	}

	// http://www.cs.iit.edu/~chen/docs/chen_sc07-dahc.pdf
	// adress = real address for global, pc for local (I think)
	void AddMiss(long diff, vector<u_int32_t>& prefetch)
	{
		Index idx = -1; //exists<PC, Index>(_indexTable, address);
		if ((idx = exists(_indexTable, diff)) < _limBuffer)
		{
			MyHistory inst(diff, -1);
			Index newIdx = addObject(inst);
			_indexTable.push_back(MyIndex(diff,  newIdx));

			if (_indexTable.size() > INDEX_SIZE)
			{
				_indexTable.pop_front();
			}

			// Statistics
			_cIndex = max(_cIndex, long(_indexTable.size()));

			return;
		}

		MyHistory hist = _historyBuffer[idx];
		Index firstIdx = idx;
		 
		while (idx >= _limBuffer)
		{
			u_int32_t next_address = diff;
			for (int j = 1; j <= 1; ++j)
			{
				MyHistory next = _historyBuffer[idx + j];
				next_address += next.first;
				u_int32_t round_address = next_address - (next_address % 16);

				if (next.second < _limBuffer)
				{
HERE:
					break;
				}

				for (int i=0; i<prefetch.size(); ++i)
					if (prefetch[i] == round_address)
						goto HERE;

				prefetch.push_back(round_address);
			}

			Index nIdx = hist.second;

			if (idx == nIdx || idx < _limBuffer)
				break;

			idx = nIdx;
			hist = _historyBuffer[idx];
		}

		MyHistory inst(diff, firstIdx);
		Index newIdx = addObject(inst);
		_indexTable.push_back(MyIndex(diff,  newIdx));

		if (_indexTable.size() > INDEX_SIZE)
		{
			_indexTable.pop_front();
		}

		// Statistics
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
			<< BUFFER_SIZE * 8 / 1024.0 << endl;
		cout << "_globalHistoryBufferStore: " << _globalHistoryBufferStore._cIndex * 8 / 1024.0 << ", "
			<< BUFFER_SIZE * 8 / 1024.0 << endl;
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
