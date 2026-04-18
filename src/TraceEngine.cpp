//*****************************************************************************
// FILE:            TraceEngine.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceEngine.h"
#include "HostResolver.h"
#include <memory>
#include <process.h>
#include <sstream>

#define TRACE_MSG(msg)                                       \
	{                                                        \
		std::wostringstream dbg(std::wostringstream::out);   \
		dbg << msg << std::endl;                             \
		OutputDebugStringW(dbg.str().c_str());               \
	}

namespace {

constexpr UCHAR IPFLAG_DONT_FRAGMENT = 0x02;

struct TraceWorkerArgs {
	TraceEngine* engine;
	int          address;
	int          ttl;
};

} // namespace

TraceEngine::TraceEngine()
	: tracing_(false)
{
}

void TraceEngine::Trace(int address, const TraceOptions& opts)
{
	if (!icmp_.IsValid()) return;

	options_ = opts;
	tracing_ = true;

	stats_.Reset();
	stats_.SetLastRemoteAddr(address);

	HANDLE threads[MAX_HOPS];
	DWORD  nthreads = 0;
	for (int i = 0; i < MAX_HOPS; ++i) {
		auto args = std::make_unique<TraceWorkerArgs>();
		args->engine  = this;
		args->address = address;
		args->ttl     = i + 1;
		const uintptr_t h = _beginthread(TraceWorkerEntry, 0, args.get());
		if (h != 0 && h != static_cast<uintptr_t>(-1)) {
			threads[nthreads++] = reinterpret_cast<HANDLE>(h);
			args.release();
		}
	}
	if (nthreads > 0)
		WaitForMultipleObjects(nthreads, threads, TRUE, INFINITE);
}

void TraceEngine::TraceWorkerEntry(void* p)
{
	std::unique_ptr<TraceWorkerArgs> args(static_cast<TraceWorkerArgs*>(p));
	args->engine->ExecuteTrace(args->address, args->ttl);
	_endthread();
}

#pragma warning(suppress: 6262) // legacy 16KB stack frame; default thread stack is 1MB
void TraceEngine::ExecuteTrace(int address, int ttl)
{
	TRACE_MSG(L"Thread with TTL=" << ttl << L" started.");

	IPINFO ipInfo;
	ipInfo.Ttl         = (UCHAR)ttl;
	ipInfo.Tos         = 0;
	ipInfo.Flags       = IPFLAG_DONT_FRAGMENT;
	ipInfo.OptionsSize = 0;
	ipInfo.OptionsData = NULL;

	char reqData[8192];
	char repData[sizeof(ICMPECHO) + 8192];
	const int nDataLen = options_.pingsize;
	for (int i = 0; i < nDataLen; ++i) reqData[i] = 32; // whitespace

	const int hop = ttl - 1;

	while (tracing_) {
		if (ttl > stats_.GetMax()) break;

		DWORD replyCount = icmp_.SendEcho(
			(u_long)address, reqData, (WORD)nDataLen, &ipInfo,
			repData, sizeof(repData), ECHO_REPLY_TIMEOUT);

		PICMPECHO reply = (PICMPECHO)repData;
		stats_.AddXmit(hop);

		if (replyCount == 0)
			continue;

		TRACE_MSG(L"TTL " << ttl << L" reply TTL " << reply->Options.Ttl
		                  << L" Status " << reply->Status
		                  << L" Reply count " << replyCount);

		switch (reply->Status) {
		case IP_SUCCESS:
		case IP_TTL_EXPIRED_TRANSIT:
			stats_.UpdateRTT(hop, reply->RoundTripTime);
			stats_.AddReturned(hop);
			if (stats_.SetAddrIfNew(hop, reply->Address) && options_.useDNS) {
				TRACE_MSG(L"Resolving DNS for hop " << hop
				                                   << L" addr " << reply->Address);
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

		if (options_.interval * 1000 > reply->RoundTripTime)
			Sleep((DWORD)(options_.interval * 1000 - reply->RoundTripTime));
	}

	TRACE_MSG(L"Thread with TTL=" << ttl << L" stopped.");
}

void TraceEngine::ResolveHopName(int hop)
{
	const int addr = stats_.GetAddr(hop);
	wchar_t hostname[NI_MAXHOST];
	if (HostResolver::ReverseResolve(addr, hostname, NI_MAXHOST)) {
		stats_.SetName(hop, hostname);
	} else {
		const auto numeric = std::format(L"{}.{}.{}.{}",
			(addr >> 24) & 0xff, (addr >> 16) & 0xff,
			(addr >>  8) & 0xff,  addr        & 0xff);
		stats_.SetName(hop, numeric.c_str());
	}
}
