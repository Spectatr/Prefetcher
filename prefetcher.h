/*
 *
 * File: prefetcher.h
 * Author: Sat Garcia (sat@cs)
 * Description: Header file for prefetcher implementation
 *
 */

#ifndef PREFETCHER_H
#define PREFETCHER_H

#define N 15

#include <sys/types.h>
#include "mem-sim.h"
#include <list>
#include <map>
using namespace std;

class my_less
{
public:
	bool operator()(Request lhd, Request rhs)
	{
		return lhd.addr < rhs.addr;
	}
};

class Prefetcher {
  private:
	map<Request, bool, my_less> _reqsMap;
	list<Request> _reqsQueue;


  public:
	Prefetcher() {}

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
