//*****************************************************************************
// FILE:            TraceEngine.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceEngine.h"
#include "HostResolver.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <thread>
#include <vector>

// Variadic so the expression chain (operator<< list) can be passed through
// verbatim; parenthesising a stream chain the way bugprone-macro-parentheses
// wants would break the left-most `wchar_t*` operand.
#define TRACE_MSG(...)                                     \
	do {                                                   \
		std::wostringstream dbg(std::wostringstream::out); \
		dbg << __VA_ARGS__ << std::endl;                   \
		OutputDebugStringW(dbg.str().c_str());             \
	} while (false)

namespace {

constexpr int    kPingRequestBufferSize = 8192;
constexpr double kIntervalMsScale       = 1000.0;
constexpr wchar_t kMsgGeneralFailure[]  = L"General failure.";

// Decoded view of a single ICMP echo reply.
struct ReplyInfo {
	ULONG     status    = IP_REQ_TIMED_OUT;
	ULONG     rtt       = 0;
	IpAddress replyAddr;
};

ReplyInfo DecodeReply(const IpAddress& dest, char* repData)
{
	ReplyInfo info;
	if (dest.family == AF_INET) {
		auto* reply   = reinterpret_cast<PICMP_ECHO_REPLY>(repData);
		info.status   = reply->Status;
		info.rtt      = reply->RoundTripTime;
		in_addr v4{};
		v4.s_addr     = reply->Address;
		info.replyAddr = IpAddress::FromIPv4(v4);
	} else {
		auto* reply   = reinterpret_cast<PICMPV6_ECHO_REPLY>(repData);
		info.status   = reply->Status;
		info.rtt      = reply->RoundTripTime;
		in6_addr v6{};
		std::memcpy(&v6, reply->Address.sin6_addr, sizeof(v6));
		info.replyAddr = IpAddress::FromIPv6(v6);
	}
	return info;
}

LPCWSTR StatusToName(ULONG status)
{
	switch (status) {
	case IP_BUF_TOO_SMALL:         return L"Reply buffer too small.";
	case IP_DEST_NET_UNREACHABLE:  return L"Destination network unreachable.";
	case IP_DEST_HOST_UNREACHABLE: return L"Destination host unreachable.";
	case IP_DEST_PROT_UNREACHABLE: return L"Destination protocol unreachable.";
	case IP_DEST_PORT_UNREACHABLE: return L"Destination port unreachable.";
	case IP_NO_RESOURCES:          return L"Insufficient IP resources were available.";
	case IP_BAD_OPTION:            return L"Bad IP option was specified.";
	case IP_HW_ERROR:              return L"Hardware error occurred.";
	case IP_PACKET_TOO_BIG:        return L"Packet was too big.";
	case IP_REQ_TIMED_OUT:         return L"Request timed out.";
	case IP_BAD_REQ:               return L"Bad request.";
	case IP_BAD_ROUTE:             return L"Bad route.";
	case IP_TTL_EXPIRED_REASSEM:   return L"The time to live expired during fragment reassembly.";
	case IP_PARAM_PROBLEM:         return L"Parameter problem.";
	case IP_SOURCE_QUENCH:         return L"Datagrams are arriving too fast to be processed and datagrams may have been discarded.";
	case IP_OPTION_TOO_BIG:        return L"An IP option was too big.";
	case IP_BAD_DESTINATION:       return L"Bad destination.";
	default:                       return kMsgGeneralFailure;
	}
}

} // namespace

TraceEngine::TraceEngine()
    : stop_event_(CreateEventW(nullptr, TRUE, FALSE, nullptr))
{
	if (stop_event_ == nullptr) {
		TRACE_MSG(L"TraceEngine: CreateEventW failed, GetLastError=" << GetLastError());
	}
}

TraceEngine::~TraceEngine()
{
	if (stop_event_ != nullptr) {
		CloseHandle(stop_event_);
	}
}

void TraceEngine::Stop()
{
	tracing_ = false;
	if (stop_event_ != nullptr) {
		SetEvent(stop_event_);
	}
}

void TraceEngine::Trace(const IpAddress& dest, const TraceOptions& opts)
{
	if (!probe_.IsValid()) {
		return;
	}
	if (dest.family != AF_INET && dest.family != AF_INET6) {
		return;
	}

	options_ = opts;
	tracing_ = true;
	if (stop_event_ != nullptr) {
		ResetEvent(stop_event_);
	}

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
		if (t.joinable()) {
			t.join();
		}
	}
}

void TraceEngine::ApplyReplyStatus(int hop, ULONG status, ULONG rtt, const IpAddress& replyAddr)
{
	if (status == IP_SUCCESS || status == IP_TTL_EXPIRED_TRANSIT) {
		stats_.UpdateRTT(hop, static_cast<int>(rtt));
		stats_.AddReturned(hop);
		if (stats_.SetAddrIfNew(hop, replyAddr) && options_.useDNS) {
			TRACE_MSG(L"Resolving DNS for hop " << hop);
			ResolveHopName(hop);
		}
	} else {
		stats_.SetName(hop, StatusToName(status));
	}
}

bool TraceEngine::WaitIntervalOrStop(ULONG rtt)
{
	const DWORD interval_ms = static_cast<DWORD>(options_.interval * kIntervalMsScale);
	if (interval_ms <= rtt) {
		return false;
	}
	const DWORD sleep_ms = interval_ms - static_cast<DWORD>(rtt);
	return stop_event_ != nullptr &&
	       WaitForSingleObject(stop_event_, sleep_ms) == WAIT_OBJECT_0;
}

#pragma warning(suppress : 6262) // legacy 16KB stack frame; default thread stack is 1MB
void TraceEngine::ExecuteTrace(const IpAddress& dest, int ttl)
{
	try {
		TRACE_MSG(L"Thread with TTL=" << ttl << L" started.");

		IcmpIO icmp;
		if (!icmp.IsValid()) {
			TRACE_MSG(L"TTL " << ttl << L" failed to initialize ICMP handle.");
			return;
		}

		char reqData[kPingRequestBufferSize];
		// Reply buffer must fit at least one reply record + the echoed payload +
		// the required ICMP headroom. For IPv6 the per-record struct is larger,
		// so size against the worst case.
		char repData[sizeof(ICMPV6_ECHO_REPLY) + kPingRequestBufferSize + ICMP_REPLY_HEADROOM];
		const int nDataLen = std::clamp(options_.pingsize, 0, static_cast<int>(sizeof(reqData)));
		for (int i = 0; i < nDataLen; ++i) {
			reqData[i] = ' '; // whitespace
		}

		const int hop = ttl - 1;

		while (tracing_ && ttl <= stats_.GetMax()) {
			const DWORD replyCount = icmp.DoEcho(
			    dest, static_cast<UCHAR>(ttl),
			    reqData, static_cast<WORD>(nDataLen),
			    repData, sizeof(repData),
			    stop_event_, ECHO_REPLY_TIMEOUT);

			stats_.AddXmit(hop);

			if (replyCount == 0) {
				continue;
			}

			const ReplyInfo info = DecodeReply(dest, repData);
			TRACE_MSG(L"TTL " << ttl << L" Status " << info.status
			                  << L" Reply count " << replyCount);
			ApplyReplyStatus(hop, info.status, info.rtt, info.replyAddr);

			if (!tracing_ || WaitIntervalOrStop(info.rtt)) {
				break;
			}
		}

		TRACE_MSG(L"Thread with TTL=" << ttl << L" stopped.");
	} catch (...) { // NOLINT(bugprone-empty-catch)
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
