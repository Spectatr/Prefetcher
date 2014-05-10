#include "prefetcher.h"
#include "mem-sim.h"

bool Prefetcher::hasRequest(u_int32_t cycle) {
	return true;
}

Request Prefetcher::getRequest(u_int32_t cycle) {
	Request req = {0};
	return req;
}

void Prefetcher::completeRequest(u_int32_t cycle) { return; }

void Prefetcher::cpuRequest(Request req) { return; }
