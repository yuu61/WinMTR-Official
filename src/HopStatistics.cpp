//*****************************************************************************
// FILE:            HopStatistics.cpp
//*****************************************************************************

#include "Global.h"
#include "HopStatistics.h"
#include <ws2tcpip.h>

void HopStatistics::Reset()
{
	std::lock_guard g(mutex_);
	host_.fill(s_nethost{});
}

void HopStatistics::SetLastRemoteAddr(int addr)
{
	std::lock_guard g(mutex_);
	last_remote_addr_ = addr;
}

int HopStatistics::GetMax() const
{
	std::lock_guard g(mutex_);
	int max = MAX_HOPS;

	for (int i = 0; i < MAX_HOPS; ++i) {
		if (host_[i].addr == last_remote_addr_) {
			max = i + 1;
			break;
		}
	}

	if (max == MAX_HOPS) {
		while ((max > 1) && (host_[max - 1].addr == host_[max - 2].addr)
		                 && (host_[max - 1].addr != 0))
			max--;
	}
	return max;
}

int HopStatistics::GetAddr(int at) const
{
	std::lock_guard g(mutex_);
	return ntohl(host_[at].addr);
}

void HopStatistics::GetName(int at, wchar_t* out) const
{
	std::lock_guard g(mutex_);
	if (host_[at].name[0] == L'\0') {
		if (host_[at].addr == 0) {
			out[0] = L'\0';
		} else {
			const int a = ntohl(host_[at].addr);
			const auto s = std::format(L"{}.{}.{}.{}",
				(a >> 24) & 0xff, (a >> 16) & 0xff,
				(a >>  8) & 0xff,  a        & 0xff);
			wcsncpy_s(out, 255, s.c_str(), _TRUNCATE);
		}
	} else {
		wcsncpy_s(out, 255, host_[at].name, _TRUNCATE);
	}
}

int HopStatistics::GetBest(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].best;
}

int HopStatistics::GetWorst(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].worst;
}

int HopStatistics::GetAvg(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].returned == 0 ? 0 : host_[at].total / host_[at].returned;
}

int HopStatistics::GetPercent(int at) const
{
	std::lock_guard g(mutex_);
	return (host_[at].xmit == 0) ? 0
	                             : (100 - (100 * host_[at].returned / host_[at].xmit));
}

int HopStatistics::GetLast(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].last;
}

int HopStatistics::GetReturned(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].returned;
}

int HopStatistics::GetXmit(int at) const
{
	std::lock_guard g(mutex_);
	return host_[at].xmit;
}

bool HopStatistics::SetAddrIfNew(int at, int addr)
{
	std::lock_guard g(mutex_);
	const bool is_new = (host_[at].addr == 0 && addr != 0);
	if (is_new) host_[at].addr = addr;
	return is_new;
}

void HopStatistics::SetName(int at, const wchar_t* name)
{
	std::lock_guard g(mutex_);
	wcsncpy_s(host_[at].name, name, _TRUNCATE);
}

void HopStatistics::UpdateRTT(int at, int rtt)
{
	std::lock_guard g(mutex_);
	host_[at].last  = rtt;
	host_[at].total += rtt;
	if (host_[at].best > rtt || host_[at].xmit == 1) host_[at].best = rtt;
	if (host_[at].worst < rtt)                       host_[at].worst = rtt;
}

void HopStatistics::AddReturned(int at)
{
	std::lock_guard g(mutex_);
	host_[at].returned++;
}

void HopStatistics::AddXmit(int at)
{
	std::lock_guard g(mutex_);
	host_[at].xmit++;
}
