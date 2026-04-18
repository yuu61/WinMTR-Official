//*****************************************************************************
// FILE:            HopStatistics.h
//
// DESCRIPTION:
//   Thread-safe per-hop statistics model. Holds the fixed-size array of hop
//   records and serializes access with an internal mutex.
//
//*****************************************************************************

#ifndef HOPSTATISTICS_H_
#define HOPSTATISTICS_H_

#include <array>
#include <mutex>

#include "IpAddress.h"

constexpr int MAX_HOPS = 30;

// INET6_ADDRSTRLEN is 46; round up for the terminator and a small margin.
constexpr size_t NETHOST_NAME_MAX = 255;

struct s_nethost {
	IpAddress     addr{};
	int           xmit = 0;
	int           returned = 0;
	unsigned long total = 0;
	int           last = 0;
	int           best = 0;
	int           worst = 0;
	wchar_t       name[NETHOST_NAME_MAX] = {};
};

class HopStatistics {
public:
	HopStatistics() = default;
	~HopStatistics() = default;
	HopStatistics(const HopStatistics&)            = delete;
	HopStatistics& operator=(const HopStatistics&) = delete;

	void Reset();
	void SetLastRemoteAddr(const IpAddress& addr);

	[[nodiscard]] int       GetMax() const;
	[[nodiscard]] IpAddress GetAddr(int at) const;
	void                    GetName(int at, wchar_t* out, size_t outSize) const;
	[[nodiscard]] int       GetBest(int at) const;
	[[nodiscard]] int       GetWorst(int at) const;
	[[nodiscard]] int       GetAvg(int at) const;
	[[nodiscard]] int       GetPercent(int at) const;
	[[nodiscard]] int       GetLast(int at) const;
	[[nodiscard]] int       GetReturned(int at) const;
	[[nodiscard]] int       GetXmit(int at) const;

	// Returns true iff the slot previously had no address and addr is
	// specified. The caller may use this to decide whether to spawn DNS
	// resolution.
	bool SetAddrIfNew(int at, const IpAddress& addr);
	void SetName(int at, const wchar_t* name);

	// Atomically updates last, total, best, and worst for one ping reply.
	void UpdateRTT(int at, int rtt);

	void AddReturned(int at);
	void AddXmit(int at);

private:
	std::array<s_nethost, MAX_HOPS> host_{};
	mutable std::mutex              mutex_;
	IpAddress                       last_remote_addr_{};
};

#endif // HOPSTATISTICS_H_
