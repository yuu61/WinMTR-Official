//*****************************************************************************
// FILE:            TraceEngine.h
//
// DESCRIPTION:
//   Orchestrates the ICMP trace lifecycle: spawns one TTL worker per hop,
//   triggers DNS resolution when new addresses appear, and holds the owning
//   IcmpIO and HopStatistics.
//
//*****************************************************************************

#ifndef TRACEENGINE_H_
#define TRACEENGINE_H_

#include "IcmpIO.h"
#include "HopStatistics.h"
#include "TraceOptions.h"
#include <atomic>

class TraceEngine {
public:
	TraceEngine();
	~TraceEngine() = default;
	TraceEngine(const TraceEngine&)            = delete;
	TraceEngine& operator=(const TraceEngine&) = delete;

	[[nodiscard]] bool    IsValid()   const { return icmp_.IsValid(); }
	[[nodiscard]] LPCWSTR LastError() const { return icmp_.LastError(); }

	[[nodiscard]] const HopStatistics& Stats() const { return stats_; }
	[[nodiscard]] HopStatistics&       Stats()       { return stats_; }

	// Blocks until every TTL worker has exited.
	void Trace(int address, const TraceOptions& opts);
	void Stop() { tracing_ = false; }

	[[nodiscard]] bool IsTracing() const { return tracing_.load(); }

private:
	void ExecuteTrace(int address, int ttl);
	void ExecuteDnsResolve(int hop);

	static void TraceWorkerEntry(void* p);
	static void DnsWorkerEntry(void* p);

	IcmpIO            icmp_;
	HopStatistics     stats_;
	TraceOptions      options_;
	std::atomic<bool> tracing_;
};

#endif // TRACEENGINE_H_
