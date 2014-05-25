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
	typedef long long DiffAddr;
	typedef int Index;
	typedef u_int64_t Address;

	typedef pair<Address, Index> HPair; 
		
	Address					_PC1;	// Last
	Address					_PC2;	// One before last

	vector<HPair>			_historyTable;
	map<pair<DiffAddr, DiffAddr>, Index>		_indexTable;

public:
	GlobalHistory() : _PC1(0), _PC2(0) {}

	void printStacks()
	{
		cout << " ======================== START ===========================\n";
		for (auto i = _indexTable.begin(); i != _indexTable.end(); ++i)
		{
			cout << "Diff: (" << i->first.first << ", " << i->first.second << ") to index: " << i->second << endl;
			//cout << "Addr: (" << i->first << ") to index: " << i->second << endl;
		}
		cout << " --------------- " << endl;
		for (int i=0; i<_historyTable.size(); ++i)
		{
			cout << "Address: " << _historyTable[i].first << " with index: " << _historyTable[i].second << endl;
		}
		cout << "\n\n\n";
	}

	void AddMiss(Address PC, Address address, vector<u_int32_t>& prefetch, bool saveFetches)
	{
		// If first time, we cannot have a diff
		if (_PC1 == 0)
		{
			_PC1 = PC;
			return;
		}

		if (_PC2 == 0)
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

		Index stepBack = -1;

		// If difference does exist in IndexTable
		if (_indexTable.count(DiffPair) > 0 && saveFetches)
		{
			// If does exist
			int limitDepth = 10;								// Depth limitation (how many backtracks)
			int limitWidth = 1;								// Width limitation (how many lookaheads)
			int index = _indexTable[DiffPair];
			stepBack = index;

			do
			{
				HPair refPair = _historyTable[index];
				for (int i=1; i<=limitWidth && index+i < _historyTable.size(); ++i)
				{
					HPair hPair = _historyTable[index + i];
					prefetch.push_back(address + hPair.first - refPair.first);
					//prefetch.push(hPair.first);
					//cout << "Prefetch: " << hPair.first << endl;
				}

				index = _historyTable[index].second;
			} 
			while(index >= 0 && limitDepth-- > 0);

			/*

			int itIndex = index;
			

			// Value of address
			HPair last = _historyTable[index];

			// Limit width
			int limitation = _historyTable.size() - 1;		// Limits to previous node (init is end of list)


			set<Address> remeber;

			while (itIndex >= 0 && (limitDepth--) > 0)
			{
				DiffAddr currDiff = 0;

				while (index < limitation && limitWidth--)
				{
					// Width
					HPair predicted = _historyTable[index + 1];
			
					// New diff
					DiffAddr newDiff = DiffAddr(predicted.first) - DiffAddr(last.first);

					// Current diff
					currDiff += newDiff;

					//if (newDiff < 0)
					//	break;

					// Insert to stack || NEED TO CHECK NO DUPLICATIONS
					Address predictedAddress = address + currDiff;

					if (remeber.count(predictedAddress) == 0)
					{
						prefetch.push(address + currDiff);
						remeber.insert(predictedAddress);
						//cout << "======>>>>>   Predicted: " << (address + currDiff);
						//cout << endl;
					}
					
					last = predicted;
					++index;
				}

				if (_historyTable[itIndex].second < 0)
				{
					break;
				}

				limitation = itIndex;
				itIndex = _historyTable[itIndex].second;
				index = itIndex;
			}
			*/
		}

		//cout << "====================\n";
		HPair hPair(address, stepBack);
		_historyTable.push_back(hPair);
		_indexTable[DiffPair] = _historyTable.size() - 1;
		if (false && stepBack != -1)
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

	u_int32_t last_address;
	u_int32_t last_address_store;
	long last_diff;
	long last_diff_store;

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
