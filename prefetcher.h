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
#include <assert.h>
using namespace std;

class GlobalHistory
{
private:
	typedef int DiffAddr;
	typedef int Index;
	typedef u_int64_t Address;

	typedef pair<Address, Index> HPair; 
		
	Address										_PC1;	// Last
	Address										_PC2;	// One before last

	vector<HPair>								_historyTable;
	map<pair<DiffAddr, DiffAddr>, Index>		_indexTable;

public:
	GlobalHistory() : _PC1(ULONG_MAX), _PC2(ULONG_MAX) {}

	void printStacks()
	{
		cout << " ======================== START ===========================\n";
		int loc = 0;
		for (auto i = _indexTable.begin(); i != _indexTable.end(); ++i)
		{
			cout << "Diff " << loc++ << ": (" << i->first.first << ", " << i->first.second << ") to index: " << i->second << endl;
			//cout << "Addr: (" << i->first << ") to index: " << i->second << endl;
		}
		cout << " --------------- " << endl;
		loc = 0;
		for (int i=0; i<_historyTable.size(); ++i)
		{
			cout << "Address " << loc++ << ": " << _historyTable[i].first << " with index: " << _historyTable[i].second << endl;
		}
		cout << "\n\n\n";
	}

	void AddMiss(Address PC, Address address, vector<u_int32_t>& prefetch, bool saveFetches)
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

		//cout << "Address and last: " << address << " ; " << _lastAddress << endl;

		// Compute current diff
		DiffAddr diffPC1 = DiffAddr(PC) - DiffAddr(_PC1);
		DiffAddr diffPC2 = DiffAddr(_PC1) - DiffAddr(_PC2);
		pair<DiffAddr, DiffAddr> DiffPair = pair<DiffAddr, DiffAddr>(diffPC1, diffPC2);

		// Forward for next time
		_PC2 = _PC1;
		_PC1 = PC;

		Index stepBack = INT_MAX;
		DiffAddr diffIt = 100;

		// If difference does exist in IndexTable
		if (_indexTable.count(DiffPair) > 0/* && saveFetches*/)
		{
			// If does exist
			int limitDepth = 10;							// Depth limitation (how many backtracks)
			int limitWidth = 3;								// Width limitation (how many lookaheads)
			int index = _indexTable[DiffPair];
			int limitation = _historyTable.size();			// limitation of width search
			stepBack = limitation - index;

			HPair hPair(address, stepBack);
			_historyTable.push_back(hPair);
			_indexTable[DiffPair] = _historyTable.size() - 1;

			long myDiff = 0;
			do
			{
				HPair refPair = _historyTable[index];
				for (int i=1; i<=limitWidth && index+i <= limitation; ++i)
				{
					HPair hPair = _historyTable[index + i];
					myDiff += (hPair.first - refPair.first);
					prefetch.push_back(address + myDiff);
					refPair = hPair;
					//prefetch.push(hPair.first);
					//cout << "Prefetch: " << hPair.first << endl;
				}

				limitation = index;
				diffIt = _historyTable[index].second;
				index -= diffIt;
			} 
			while(index >= 0 && diffIt > 1 && limitDepth-- > 0);
		}
		else
		{
			HPair hPair(address, stepBack);
			_historyTable.push_back(hPair);
			_indexTable[DiffPair] = _historyTable.size() - 1;
		}
		//cout << "====================\n";

		if (false && stepBack != INT_MAX)
		{
			printStacks();
			cout << "PREDICTS:        ================\n";
			cout << prefetch.back() << endl;
			system("pause");
		}
		//cout << "Adding " << diffAddr << endl;
		//cout << "====================\n";
		//cout << "";
		// Here add implementation for memory limitation
		return;
	}
};


class Prefetcher {
  private:
	//queue<u_int32_t> _reqQueue;
	GlobalHistory _globalHistoryLoads;
	GlobalHistory _globalHistoryStores;

	
	queue<u_int32_t> _fetchQueue;
	set<u_int32_t> _reqsMap;

	long _cFetch;
	long _cReqs;

	u_int32_t _last_address_load;
	u_int32_t _last_address_store;

	long _address_load_diff;
	long _address_store_diff;

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
