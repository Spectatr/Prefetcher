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
#include <set>
#include <list>
#include <assert.h>
#include <limits.h>
using namespace std;

#define BUFFER_SIZE 128
#define INDEX_SIZE 16

class GlobalHistory
{
private:
	typedef u_int64_t Address;
	typedef int DiffAddr;
	typedef int Index;

	typedef pair<
		pair<DiffAddr, DiffAddr>, Index>	IPair;
	typedef pair<Address, DiffAddr>			HPair; 
	typedef list<HPair>::iterator			HistoryIt;
	
	Address									_PC1;	// Last
	Address									_PC2;	// One before last

	list<HPair>								_historyTable;
	list<IPair>								_indexTable;
	Index									_sizeLimit;

public:

	static char index1, index2;

private:

	Index ListFindAndRemove(pair<DiffAddr, DiffAddr>& thePair)
	{
		//cout << "Looking for " << thePair.first << " , " << thePair.second << endl;

		list<IPair>::iterator myIter = _indexTable.begin();

		for (; myIter != _indexTable.end(); ++myIter)
		{
			if (myIter->first == thePair)
			{
				Index idx = myIter->second;
				_indexTable.erase(myIter);

				return idx - _sizeLimit;
			}
		}

		return -1;
	}

	void ListAddLimited(IPair toPush)
	{
		_indexTable.push_back(toPush);

		if (_indexTable.size() > INDEX_SIZE)
			_indexTable.pop_front();
	}

public:
	GlobalHistory() : _PC1(ULONG_MAX), _PC2(ULONG_MAX), _sizeLimit(0) {}

	void PrintStacks()
	{
		cout << " ======================== START (index) ===========================\n";
		int loc = 0;
		for (list<IPair>::iterator i = _indexTable.begin(); i != _indexTable.end(); ++i)
		{
			cout << "Diff " << loc++ << ": (" << i->first.first << ", " << i->first.second << ") to index: " << i->second << endl;
		}
		cout << " ---------(history) ------ " << endl;
		loc = 0;
		for (HistoryIt it = _historyTable.begin(); it != _historyTable.end(); ++it)
		{
			cout << "Address " << loc++ << ": " << it->first << " with index: " << it->second << endl;
		}
		cout << "\n\n\n";
	}

	void AddMiss(Address PC, Address address, queue<u_int32_t>& prefetch, int n)
	{
		// If first time, we cannot have a diff
		if (_PC1 == ULONG_MAX)
		{
			_PC1 = PC;
			return;
		}

		if (_PC2 == ULONG_MAX)
		{
			_PC2 = _PC1;
			_PC1 = PC;
			return;
		}

		// Compute current diff
		DiffAddr diffPC1 = DiffAddr(PC) + DiffAddr(_PC1);
		DiffAddr diffPC2 = DiffAddr(_PC1) + DiffAddr(_PC2);
		pair<DiffAddr, DiffAddr> DiffPair = pair<DiffAddr, DiffAddr>(diffPC1, diffPC2);

		// Forward for next time
		_PC2 = _PC1;
		_PC1 = PC;

		Index stepBack = INT_MAX;
		Index index;

		// If difference does exist in IndexTable
		if ((index = ListFindAndRemove(DiffPair)) >= 0)
		{
			int limitation = _historyTable.size();			// limitation of width search
			stepBack = limitation - index + 1;

			// If does exist
			short limitDepth = index1;							// Depth limitation (how many backtracks)
			short limitWidth = index2;								// Width limitation (how many lookaheads)

			HPair hPair(address, stepBack);
			_historyTable.push_back(hPair);
			ListAddLimited(IPair(DiffPair,  _sizeLimit + _historyTable.size() - 1));

			if (_historyTable.size() > BUFFER_SIZE)
			{
				_historyTable.pop_front();
				++_sizeLimit;
			}

			DiffAddr diffIt = stepBack;
			HistoryIt historyHit = _historyTable.end();

			while(index >= 0 && diffIt >= 1 && limitDepth-- > 0)
			{
				DiffAddr myDiff = 0;

				std::advance(historyHit, -diffIt);

				HPair refPair = *historyHit; 
				HPair orgPair = refPair;

				int i=1;
				for (; i<=limitWidth && index+i <= limitation && (n-- > 0); ++i)
				{
					std::advance(historyHit, 1);
					HPair hPair = *historyHit;
					myDiff += (DiffAddr(hPair.first) - DiffAddr(refPair.first));

					prefetch.push(Address(DiffAddr(address) + myDiff));
					refPair = hPair;
				}

				std::advance(historyHit, 1-i);

				limitation = index;
				diffIt = orgPair.second;
				index -= diffIt;
			} 

			return;
		}
	
		HPair hPair(address, stepBack);
		_historyTable.push_back(hPair);
		ListAddLimited(IPair(DiffPair,  _sizeLimit + _historyTable.size() - 1));

		if (_historyTable.size() > BUFFER_SIZE)
		{
			_historyTable.pop_front();
			++_sizeLimit;
		}
		return;
	}
};


class Prefetcher {
  private:
	//queue<u_int32_t> _reqQueue;
	GlobalHistory _globalHistoryLoads;
	GlobalHistory _globalHistoryStores;

	
	queue<u_int32_t> _fetchQueue;

	long _cFetch;
	long _cReqs;

	long _address_load_diff;
	long _address_store_diff;

	u_int32_t _last_reset_cycle;
	char	  _count_to_cycle;

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
