//*****************************************************************************
// FILE:            TraceEngine.h
//
// DESCRIPTION:
//   Orchestrates the ICMP trace lifecycle: spawns one TTL worker per hop,
//   triggers DNS resolution when new addresses appear, and holds the shared
//   HopStatistics model.
//
//*****************************************************************************

#ifndef TRACEENGINE_H_
#define TRACEENGINE_H_

#include "IcmpIO.h"
#include "HopStatistics.h"
#include "TraceOptions.h"
#include "IpAddress.h"

#include <atomic>
#include <thread>
#include <vector>

class TraceEngine {
public:
	TraceEngine();
	~TraceEngine();
	TraceEngine(const TraceEngine&)            = delete;
	TraceEngine& operator=(const TraceEngine&) = delete;

	[[nodiscard]] bool    IsValid()   const { return probe_.IsValid(); }
	[[nodiscard]] LPCWSTR LastError() const { return probe_.LastError(); }

	[[nodiscard]] const HopStatistics& Stats() const { return stats_; }
	[[nodiscard]] HopStatistics&       Stats()       { return stats_; }

	// Blocks until every TTL worker has exited.
	void Trace(const IpAddress& dest, const TraceOptions& opts);
	void Stop();

	[[nodiscard]] bool IsTracing() const { return tracing_.load(); }

private:
	void ExecuteTrace(const IpAddress& dest, int ttl);
	void ResolveHopName(int hop);

	// Used only to validate that ICMP is usable on this host.
	IcmpIO            probe_;
	HopStatistics     stats_;
	TraceOptions      options_;
	std::atomic<bool> tracing_;
	HANDLE            stop_event_;
};

#endif // TRACEENGINE_H_
