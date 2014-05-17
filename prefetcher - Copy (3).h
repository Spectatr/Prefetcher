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
#include <cmath>

using namespace std;

#define N 100

class GradientDescent
{
private:
	double _weights[N];
	
	u_int32_t _observations[N];
	bool _starting;
	short _index;
	short _boop;
	
public:
	GradientDescent() : _starting(true), _index(-1), _boop(1)
	{
		for (int i = 0; i < N; ++i)
		{
			_weights[i] = 0;
			_observations[i] = 0;
		}
	}

	bool IsReady()
	{
		return !_starting;
	}

	void AddObservation(u_int32_t address, bool isHit)
	{
		_boop = 1;
		// Warmp up _ns steps of observations
		short lastIndex = _index;
		_index = (_index + 1) % N;		
		_observations[_index] = address;

		if (_starting && _index == N - 1)
		{
			_starting = false;
			return;
		}

		long offset = (_observations[_index] - _observations[lastIndex]);

		if (isHit)
		{
			double evalFun = 0;
			for (int j = 0; j < N; ++j)
			{
				evalFun += _weights[j] * _observations[(_index + j + N - 1) % N];
			}

			if (offset < evalFun)
			{
				_weights[_index] += 0.00000001;
			}
			else if (offset > evalFun)
			{
				_weights[_index] -= 0.00000001;
			}
		}
	}

	long PredictOffset()
	{
		double evalFun = 0;
		for (int j = 0; j < N; ++j)
		{
			evalFun += _weights[j] * _observations[(_index + j) % N];
		}
		
		evalFun = ceil(evalFun / 16.0) * 16;
		//cout << evalFun << endl;

		return long(evalFun);
	}

	u_int32_t LastAddress()
	{
		return _observations[(_index - _boop++) % N];
	}
};

class PrefetchLasts
{
private:
	typedef tuple<u_int32_t, u_int32_t, u_int32_t, u_int32_t> History;
	map<History, u_int32_t> _predictMap;
	u_int32_t _lasts[5];
	short _idx;

public:
	PrefetchLasts() : _idx(0) { _lasts[4] = 0; }

	bool IsReady()
	{
		return (_lasts[4] != 0);
	}

	void AddAddress(u_int32_t address)
	{
		_lasts[_idx] = address;
		
		if (IsReady())
		{
			auto t = make_tuple(_lasts[(_idx + 1) % 5], _lasts[(_idx + 2) % 5],
				_lasts[(_idx + 3) % 5], _lasts[(_idx + 4) % 5]);

			_predictMap[t] = address;
		}

		_idx = (_idx + 1) % 5;
	}

	void PrefetchSome(queue<u_int32_t>& prefech)
	{
		u_int32_t add1 = _lasts[(_idx + 2) % 5], 
				  add2 = _lasts[(_idx + 3) % 5],
				  add3 = _lasts[(_idx + 4) % 5],
				  add4 = _lasts[_idx];

		auto tp = make_tuple(add1, add2, add3, add4);

		for (int i = 0; i < 15; ++i)
		{
			if (_predictMap.find(tp) == _predictMap.end())
			{
				break;
			}

			u_int32_t addpred = _predictMap[tp];
			prefech.push(addpred);

			add1 = add2;
			add2 = add3;
			add3 = add4;
			add4 = addpred;
		}			
	}
};


class Prefetcher {
  private:
	  PrefetchLasts _prefetchLast;
	  queue<u_int32_t> _prefech;
	

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
