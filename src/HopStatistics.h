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

constexpr int MAX_HOPS = 30;

struct s_nethost {
	int addr = 0;                  // IP stored in network byte order
	int xmit = 0;
	int returned = 0;
	unsigned long total = 0;
	int last = 0;
	int best = 0;
	int worst = 0;
	wchar_t name[255] = {};
};

class HopStatistics {
public:
	HopStatistics() = default;
	~HopStatistics() = default;
	HopStatistics(const HopStatistics&)            = delete;
	HopStatistics& operator=(const HopStatistics&) = delete;

	void Reset();
	void SetLastRemoteAddr(int addr);

	[[nodiscard]] int GetMax() const;
	[[nodiscard]] int GetAddr(int at) const;
	void              GetName(int at, wchar_t* out) const;
	[[nodiscard]] int GetBest(int at) const;
	[[nodiscard]] int GetWorst(int at) const;
	[[nodiscard]] int GetAvg(int at) const;
	[[nodiscard]] int GetPercent(int at) const;
	[[nodiscard]] int GetLast(int at) const;
	[[nodiscard]] int GetReturned(int at) const;
	[[nodiscard]] int GetXmit(int at) const;

	// Returns true iff the slot previously had no address and addr is non-zero.
	// The caller may use this to decide whether to spawn DNS resolution.
	bool SetAddrIfNew(int at, int addr);
	void SetName(int at, const wchar_t* name);

	// Atomically updates last, total, best, and worst for one ping reply.
	void UpdateRTT(int at, int rtt);

	void AddReturned(int at);
	void AddXmit(int at);

private:
	std::array<s_nethost, MAX_HOPS> host_{};
	mutable std::mutex              mutex_;
	int                             last_remote_addr_ = 0;
};

#endif // HOPSTATISTICS_H_
