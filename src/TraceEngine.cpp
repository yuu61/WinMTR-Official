//*****************************************************************************
// FILE:            TraceEngine.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceEngine.h"
#include "HostResolver.h"

#include <algorithm>
#include <sstream>
#include <thread>
#include <vector>

#define TRACE_MSG(msg)                                       \
	{                                                        \
		std::wostringstream dbg(std::wostringstream::out);   \
		dbg << msg << std::endl;                             \
		OutputDebugStringW(dbg.str().c_str());               \
	}

TraceEngine::TraceEngine()
	: tracing_(false),
	  stop_event_(NULL)
{
	stop_event_ = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (stop_event_ == NULL) {
		TRACE_MSG(L"TraceEngine: CreateEventW failed, GetLastError=" << GetLastError());
	}
}

TraceEngine::~TraceEngine()
{
	if (stop_event_ != NULL) CloseHandle(stop_event_);
}

void TraceEngine::Stop()
{
	tracing_ = false;
	if (stop_event_ != NULL) SetEvent(stop_event_);
}

void TraceEngine::Trace(const IpAddress& dest, const TraceOptions& opts)
{
	if (!probe_.IsValid()) return;
	if (dest.family != AF_INET && dest.family != AF_INET6) return;

	options_ = opts;
	tracing_ = true;
	if (stop_event_ != NULL) ResetEvent(stop_event_);

	stats_.Reset();
	stats_.SetLastRemoteAddr(dest);

	std::vector<std::thread> workers;
	workers.reserve(MAX_HOPS);
	for (int i = 0; i < MAX_HOPS; ++i) {
		const int ttl = i + 1;
		try {
			workers.emplace_back([this, dest, ttl] { ExecuteTrace(dest, ttl); });
		} catch (const std::system_error&) {
			// Thread creation failed; stop spawning but let already-started
			// workers finish.
			break;
		}
	}

	for (auto& t : workers) {
		if (t.joinable()) t.join();
	}
}

#pragma warning(suppress: 6262) // legacy 16KB stack frame; default thread stack is 1MB
void TraceEngine::ExecuteTrace(const IpAddress& dest, int ttl)
{
	try {
	TRACE_MSG(L"Thread with TTL=" << ttl << L" started.");

	IcmpIO icmp;
	if (!icmp.IsValid()) {
		TRACE_MSG(L"TTL " << ttl << L" failed to initialize ICMP handle.");
		return;
	}

	char reqData[8192];
	// Reply buffer must fit at least one reply record + the echoed payload +
	// the required ICMP headroom. For IPv6 the per-record struct is larger,
	// so size against the worst case.
	char repData[sizeof(ICMPV6_ECHO_REPLY) + 8192 + ICMP_REPLY_HEADROOM];
	const int nDataLen = std::clamp(options_.pingsize, 0, static_cast<int>(sizeof(reqData)));
	for (int i = 0; i < nDataLen; ++i) reqData[i] = ' '; // whitespace

	const int hop = ttl - 1;

	while (tracing_) {
		if (ttl > stats_.GetMax()) break;

		DWORD replyCount = icmp.DoEcho(
			dest, static_cast<UCHAR>(ttl),
			reqData, static_cast<WORD>(nDataLen),
			repData, sizeof(repData),
			stop_event_, ECHO_REPLY_TIMEOUT);

		stats_.AddXmit(hop);

		if (replyCount == 0) {
			if (!tracing_) break;
			continue;
		}

		ULONG     status   = IP_REQ_TIMED_OUT;
		ULONG     rtt      = 0;
		IpAddress replyAddr;

		if (dest.family == AF_INET) {
			auto* reply = reinterpret_cast<PICMP_ECHO_REPLY>(repData);
			status = reply->Status;
			rtt    = reply->RoundTripTime;
			in_addr v4{};
			v4.s_addr = reply->Address;
			replyAddr = IpAddress::FromIPv4(v4);
		} else {
			auto* reply = reinterpret_cast<PICMPV6_ECHO_REPLY>(repData);
			status = reply->Status;
			rtt    = reply->RoundTripTime;
			in6_addr v6{};
			memcpy(&v6, reply->Address.sin6_addr, sizeof(v6));
			replyAddr = IpAddress::FromIPv6(v6);
		}

		TRACE_MSG(L"TTL " << ttl << L" Status " << status
		                  << L" Reply count " << replyCount);

		switch (status) {
		case IP_SUCCESS:
		case IP_TTL_EXPIRED_TRANSIT:
			stats_.UpdateRTT(hop, static_cast<int>(rtt));
			stats_.AddReturned(hop);
			if (stats_.SetAddrIfNew(hop, replyAddr) && options_.useDNS) {
				TRACE_MSG(L"Resolving DNS for hop " << hop);
				ResolveHopName(hop);
			}
			break;
		case IP_BUF_TOO_SMALL:         stats_.SetName(hop, L"Reply buffer too small."); break;
		case IP_DEST_NET_UNREACHABLE:  stats_.SetName(hop, L"Destination network unreachable."); break;
		case IP_DEST_HOST_UNREACHABLE: stats_.SetName(hop, L"Destination host unreachable."); break;
		case IP_DEST_PROT_UNREACHABLE: stats_.SetName(hop, L"Destination protocol unreachable."); break;
		case IP_DEST_PORT_UNREACHABLE: stats_.SetName(hop, L"Destination port unreachable."); break;
		case IP_NO_RESOURCES:          stats_.SetName(hop, L"Insufficient IP resources were available."); break;
		case IP_BAD_OPTION:            stats_.SetName(hop, L"Bad IP option was specified."); break;
		case IP_HW_ERROR:              stats_.SetName(hop, L"Hardware error occurred."); break;
		case IP_PACKET_TOO_BIG:        stats_.SetName(hop, L"Packet was too big."); break;
		case IP_REQ_TIMED_OUT:         stats_.SetName(hop, L"Request timed out."); break;
		case IP_BAD_REQ:               stats_.SetName(hop, L"Bad request."); break;
		case IP_BAD_ROUTE:             stats_.SetName(hop, L"Bad route."); break;
		case IP_TTL_EXPIRED_REASSEM:   stats_.SetName(hop, L"The time to live expired during fragment reassembly."); break;
		case IP_PARAM_PROBLEM:         stats_.SetName(hop, L"Parameter problem."); break;
		case IP_SOURCE_QUENCH:         stats_.SetName(hop, L"Datagrams are arriving too fast to be processed and datagrams may have been discarded."); break;
		case IP_OPTION_TOO_BIG:        stats_.SetName(hop, L"An IP option was too big."); break;
		case IP_BAD_DESTINATION:       stats_.SetName(hop, L"Bad destination."); break;
		case IP_GENERAL_FAILURE:       stats_.SetName(hop, L"General failure."); break;
		default:                       stats_.SetName(hop, L"General failure."); break;
		}

		if (!tracing_) break;

		const DWORD interval_ms = static_cast<DWORD>(options_.interval * 1000);
		if (interval_ms > rtt) {
			const DWORD sleep_ms = interval_ms - static_cast<DWORD>(rtt);
			if (stop_event_ != NULL &&
			    WaitForSingleObject(stop_event_, sleep_ms) == WAIT_OBJECT_0) {
				break;
			}
		}
	}

	TRACE_MSG(L"Thread with TTL=" << ttl << L" stopped.");
	} catch (...) {
		TRACE_MSG(L"ExecuteTrace caught exception on TTL " << ttl);
	}
}

void TraceEngine::ResolveHopName(int hop)
{
	const IpAddress addr = stats_.GetAddr(hop);
	wchar_t hostname[NI_MAXHOST];
	if (HostResolver::ReverseResolve(addr, hostname, NI_MAXHOST)) {
		stats_.SetName(hop, hostname);
	} else {
		wchar_t numeric[INET6_ADDRSTRLEN];
		HostResolver::FormatNumeric(addr, numeric, INET6_ADDRSTRLEN);
		stats_.SetName(hop, numeric);
	}
}
