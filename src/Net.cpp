//*****************************************************************************
// FILE:            Net.cpp
//
//*****************************************************************************
#include "Global.h"
#include "Net.h"
#include "HostResolver.h"
#include <iostream>
#include <memory>
#include <sstream>

#define TRACE_MSG(msg)										\
	{														\
	std::wostringstream dbg_msg(std::wostringstream::out);	\
	dbg_msg << msg << std::endl;							\
	OutputDebugStringW(dbg_msg.str().c_str());				\
	}

constexpr UCHAR IPFLAG_DONT_FRAGMENT = 0x02;
constexpr int   MAX_HOPS             = 30;

struct trace_thread {
	int			address;
	WinMTRNet	*winmtr;
	int			ttl;
};

struct dns_resolver_thread {
	int			index;
	WinMTRNet	*winmtr;
};

struct ping_thread_args {
	WinMTRNet*   net;
	HANDLE       mutex;
	std::wstring hostname;
	TraceOptions opts;
};

void TraceThread(void *p);
void DnsResolverThread(void *p);
void PingThreadWorker(void *p);

WinMTRNet::WinMTRNet() {

	ghMutex = CreateMutex(NULL, FALSE, NULL);
	tracing=false;
	initialized = false;
	options.pingsize = DEFAULT_PING_SIZE;
	options.interval = DEFAULT_INTERVAL;
	options.useDNS = DEFAULT_DNS;
	WSADATA wsaData;

    if( WSAStartup(MAKEWORD(2, 2), &wsaData) ) {
        AfxMessageBox(L"Failed initializing windows sockets library!");
		return;
    }

    hICMP_DLL =  LoadLibraryW(L"ICMP.DLL");
    if (hICMP_DLL == 0) {
        AfxMessageBox(L"Failed: Unable to locate ICMP.DLL!");
        return;
    }

    /*
     * Get pointers to ICMP.DLL functions
     */
    lpfnIcmpCreateFile  = (LPFNICMPCREATEFILE)GetProcAddress(hICMP_DLL,"IcmpCreateFile");
    lpfnIcmpCloseHandle = (LPFNICMPCLOSEHANDLE)GetProcAddress(hICMP_DLL,"IcmpCloseHandle");
    lpfnIcmpSendEcho    = (LPFNICMPSENDECHO)GetProcAddress(hICMP_DLL,"IcmpSendEcho");
    if ((!lpfnIcmpCreateFile) || (!lpfnIcmpCloseHandle) || (!lpfnIcmpSendEcho)) {
        AfxMessageBox(L"Wrong ICMP.DLL system library !");
        return;
    }

    /*
     * IcmpCreateFile() - Open the ping service
     */
    hICMP = (HANDLE) lpfnIcmpCreateFile();
    if (hICMP == INVALID_HANDLE_VALUE) {
        AfxMessageBox(L"Error in ICMP.DLL !");
        return;
    }

	ResetHops();

	initialized = true;
	return;
}

WinMTRNet::~WinMTRNet()
{
	if(initialized) {
		/*
		 * IcmpCloseHandle - Close the ICMP handle
		 */
		lpfnIcmpCloseHandle(hICMP);

		// Shut down...
		FreeLibrary(hICMP_DLL);

		WSACleanup();

		CloseHandle(ghMutex);
	}
}

void WinMTRNet::ResetHops()
{
	host.fill(s_nethost{});
}

void WinMTRNet::DoTrace(int address, const TraceOptions& opts)
{
	HANDLE hThreads[MAX_HOPS];
	options = opts;
	tracing = true;

	ResetHops();

	last_remote_addr = address;

	// one thread per TTL value
	for(int i = 0; i < MAX_HOPS; i++) {
		auto current = std::make_unique<trace_thread>();
		current->address = address;
		current->winmtr  = this;
		current->ttl     = i + 1;
		const uintptr_t h = _beginthread(TraceThread, 0, current.get());
		if (h == 0 || h == static_cast<uintptr_t>(-1)) {
			// thread creation failed: ownership stays with unique_ptr and is freed at scope exit
			hThreads[i] = INVALID_HANDLE_VALUE;
		} else {
			hThreads[i] = reinterpret_cast<HANDLE>(h);
			current.release();  // ownership transferred to TraceThread
		}
	}

	WaitForMultipleObjects(MAX_HOPS, hThreads, TRUE, INFINITE);
}

void WinMTRNet::StopTrace()
{
	tracing = false;
}

void WinMTRNet::BeginTraceAsync(const std::wstring& hostname, const TraceOptions& opts, HANDLE externalMutex)
{
	auto args = std::make_unique<ping_thread_args>();
	args->net      = this;
	args->mutex    = externalMutex;
	args->hostname = hostname;
	args->opts     = opts;
	const uintptr_t h = _beginthread(PingThreadWorker, 0, args.get());
	if (h != 0 && h != static_cast<uintptr_t>(-1)) {
		args.release();  // ownership transferred to PingThreadWorker
	}
}

void PingThreadWorker(void* p)
{
	std::unique_ptr<ping_thread_args> args(static_cast<ping_thread_args*>(p));
	WaitForSingleObject(args->mutex, INFINITE);

	int traddr;
	CString err;
	if (!WinMTRHostResolver::Resolve(args->hostname.c_str(), traddr, err)) {
		AfxMessageBox(err);
	} else {
		args->net->DoTrace(traddr, args->opts);
	}

	ReleaseMutex(args->mutex);
	_endthread();
}

#pragma warning(suppress: 6262) // legacy 16KB stack frame; default thread stack is 1MB
void TraceThread(void *p)
{
	std::unique_ptr<trace_thread> current(static_cast<trace_thread*>(p));
	WinMTRNet *wmtrnet = current->winmtr;
	TRACE_MSG(L"Threaad with TTL=" << current->ttl << L" started.");

    IPINFO			stIPInfo, *lpstIPInfo;
    DWORD			dwReplyCount;
	char			achReqData[8192];
	int				nDataLen									= wmtrnet->options.pingsize;
	char			achRepData[sizeof(ICMPECHO) + 8192];


    /*
     * Init IPInfo structure
     */
    lpstIPInfo				= &stIPInfo;
    stIPInfo.Ttl			= (UCHAR)current->ttl;
    stIPInfo.Tos			= 0;
    stIPInfo.Flags			= IPFLAG_DONT_FRAGMENT;
    stIPInfo.OptionsSize	= 0;
    stIPInfo.OptionsData	= NULL;

    for (int i=0; i<nDataLen; i++) achReqData[i] = 32; //whitespaces

    while(wmtrnet->tracing) {

		// For some strange reason, ICMP API is not filling the TTL for icmp echo reply
		// Check if the current thread should be closed
		if( current->ttl > wmtrnet->GetMax() ) break;

		// NOTE: some servers does not respond back everytime, if TTL expires in transit; e.g. :
		// ping -n 20 -w 5000 -l 64 -i 7 www.chinapost.com.tw  -> less that half of the replies are coming back from 219.80.240.93
		// but if we are pinging ping -n 20 -w 5000 -l 64 219.80.240.93  we have 0% loss
		// A resolution would be:
		// - as soon as we get a hop, we start pinging directly that hop, with a greater TTL
		// - a drawback would be that, some servers are configured to reply for TTL transit expire, but not to ping requests, so,
		// for these servers we'll have 100% loss
		dwReplyCount = wmtrnet->lpfnIcmpSendEcho(wmtrnet->hICMP, current->address, achReqData, (WORD)nDataLen, lpstIPInfo, achRepData, sizeof(achRepData), ECHO_REPLY_TIMEOUT);

		PICMPECHO icmp_echo_reply = (PICMPECHO)achRepData;

		wmtrnet->AddXmit(current->ttl - 1);
		if (dwReplyCount != 0) {
			TRACE_MSG(L"TTL " << current->ttl << L" reply TTL " << icmp_echo_reply->Options.Ttl << L" Status " << icmp_echo_reply->Status << L" Reply count " << dwReplyCount);

			switch(icmp_echo_reply->Status) {
				case IP_SUCCESS:
				case IP_TTL_EXPIRED_TRANSIT:
					wmtrnet->SetLast(current->ttl - 1, icmp_echo_reply->RoundTripTime);
					wmtrnet->SetBest(current->ttl - 1, icmp_echo_reply->RoundTripTime);
					wmtrnet->SetWorst(current->ttl - 1, icmp_echo_reply->RoundTripTime);
					wmtrnet->AddReturned(current->ttl - 1);
					wmtrnet->SetAddr(current->ttl - 1, icmp_echo_reply->Address);
				break;
				case IP_BUF_TOO_SMALL:
					wmtrnet->SetName(current->ttl - 1, L"Reply buffer too small.");
				break;
				case IP_DEST_NET_UNREACHABLE:
					wmtrnet->SetName(current->ttl - 1, L"Destination network unreachable.");
				break;
				case IP_DEST_HOST_UNREACHABLE:
					wmtrnet->SetName(current->ttl - 1, L"Destination host unreachable.");
				break;
				case IP_DEST_PROT_UNREACHABLE:
					wmtrnet->SetName(current->ttl - 1, L"Destination protocol unreachable.");
				break;
				case IP_DEST_PORT_UNREACHABLE:
					wmtrnet->SetName(current->ttl - 1, L"Destination port unreachable.");
				break;
				case IP_NO_RESOURCES:
					wmtrnet->SetName(current->ttl - 1, L"Insufficient IP resources were available.");
				break;
				case IP_BAD_OPTION:
					wmtrnet->SetName(current->ttl - 1, L"Bad IP option was specified.");
				break;
				case IP_HW_ERROR:
					wmtrnet->SetName(current->ttl - 1, L"Hardware error occurred.");
				break;
				case IP_PACKET_TOO_BIG:
					wmtrnet->SetName(current->ttl - 1, L"Packet was too big.");
				break;
				case IP_REQ_TIMED_OUT:
					wmtrnet->SetName(current->ttl - 1, L"Request timed out.");
				break;
				case IP_BAD_REQ:
					wmtrnet->SetName(current->ttl - 1, L"Bad request.");
				break;
				case IP_BAD_ROUTE:
					wmtrnet->SetName(current->ttl - 1, L"Bad route.");
				break;
				case IP_TTL_EXPIRED_REASSEM:
					wmtrnet->SetName(current->ttl - 1, L"The time to live expired during fragment reassembly.");
				break;
				case IP_PARAM_PROBLEM:
					wmtrnet->SetName(current->ttl - 1, L"Parameter problem.");
				break;
				case IP_SOURCE_QUENCH:
					wmtrnet->SetName(current->ttl - 1, L"Datagrams are arriving too fast to be processed and datagrams may have been discarded.");
				break;
				case IP_OPTION_TOO_BIG:
					wmtrnet->SetName(current->ttl - 1, L"An IP option was too big.");
				break;
				case IP_BAD_DESTINATION:
					wmtrnet->SetName(current->ttl - 1, L"Bad destination.");
				break;
				case IP_GENERAL_FAILURE:
					wmtrnet->SetName(current->ttl - 1, L"General failure.");
				break;
				default:
					wmtrnet->SetName(current->ttl - 1, L"General failure.");
			}

			if(wmtrnet->options.interval * 1000 > icmp_echo_reply->RoundTripTime)
				Sleep((DWORD)(wmtrnet->options.interval * 1000 - icmp_echo_reply->RoundTripTime));
		}

    } /* end ping loop */

	TRACE_MSG(L"Thread with TTL=" << current->ttl << L" stopped.");

	_endthread();
}

int WinMTRNet::GetAddr(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int addr = ntohl(host[at].addr);
	ReleaseMutex(ghMutex);
	return addr;
}

int WinMTRNet::GetName(int at, wchar_t *n)
{
	WaitForSingleObject(ghMutex, INFINITE);
	if (host[at].name[0] == L'\0') {
		const int addr = GetAddr(at);
		if (addr == 0) {
			n[0] = L'\0';
		} else {
			const auto s = std::format(L"{}.{}.{}.{}",
				(addr >> 24) & 0xff, (addr >> 16) & 0xff,
				(addr >> 8) & 0xff,  addr & 0xff);
			wcsncpy_s(n, 255, s.c_str(), _TRUNCATE);
		}
	} else {
		wcsncpy_s(n, 255, host[at].name, _TRUNCATE);
	}
	ReleaseMutex(ghMutex);
	return 0;
}

int WinMTRNet::GetBest(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].best;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetWorst(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].worst;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetAvg(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].returned == 0 ? 0 : host[at].total / host[at].returned;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetPercent(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = (host[at].xmit == 0) ? 0 : (100 - (100 * host[at].returned / host[at].xmit));
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetLast(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].last;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetReturned(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].returned;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetXmit(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	int ret = host[at].xmit;
	ReleaseMutex(ghMutex);
	return ret;
}

int WinMTRNet::GetMax()
{
	WaitForSingleObject(ghMutex, INFINITE);
	int max = MAX_HOPS;

	// first match: traced address responds on ping requests, and the address is in the hosts list
	for(int i = 0; i < MAX_HOPS; i++) {
		if(host[i].addr == last_remote_addr) {
			max = i + 1;
			break;
		}
	}

	// second match:  traced address doesn't responds on ping requests
	if(max == MAX_HOPS) {
		while((max > 1) && (host[max - 1].addr == host[max - 2].addr) && (host[max - 1].addr != 0) ) max--;
	}

	ReleaseMutex(ghMutex);
	return max;
}

void WinMTRNet::SetAddr(int at, __int32 addr)
{
	WaitForSingleObject(ghMutex, INFINITE);
	if(host[at].addr == 0 && addr != 0) {
		TRACE_MSG(L"Start DnsResolverThread for new address " << addr << L". Old addr value was " << host[at].addr);
		host[at].addr = addr;
		if (options.useDNS) {
			auto dnt = std::make_unique<dns_resolver_thread>();
			dnt->index  = at;
			dnt->winmtr = this;
			const uintptr_t h = _beginthread(DnsResolverThread, 0, dnt.get());
			if (h != 0 && h != static_cast<uintptr_t>(-1)) {
				dnt.release();  // ownership transferred to DnsResolverThread
			}
		}
	}

	ReleaseMutex(ghMutex);
}

void WinMTRNet::SetName(int at, const wchar_t *n)
{
	WaitForSingleObject(ghMutex, INFINITE);
	wcsncpy_s(host[at].name, n, _TRUNCATE);
	ReleaseMutex(ghMutex);
}

void WinMTRNet::SetBest(int at, int current)
{
	WaitForSingleObject(ghMutex, INFINITE);
	if(host[at].best > current || host[at].xmit == 1) {
		host[at].best = current;
	};
	if(host[at].worst < current) {
		host[at].worst = current;
	}

	ReleaseMutex(ghMutex);
}

void WinMTRNet::SetWorst(int at, int current)
{
	UNREFERENCED_PARAMETER(at);
	UNREFERENCED_PARAMETER(current);
	WaitForSingleObject(ghMutex, INFINITE);
	ReleaseMutex(ghMutex);
}

void WinMTRNet::SetLast(int at, int last)
{
	WaitForSingleObject(ghMutex, INFINITE);
	host[at].last = last;
	host[at].total += last;
	ReleaseMutex(ghMutex);
}

void WinMTRNet::AddReturned(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	host[at].returned++;
	ReleaseMutex(ghMutex);
}

void WinMTRNet::AddXmit(int at)
{
	WaitForSingleObject(ghMutex, INFINITE);
	host[at].xmit++;
	ReleaseMutex(ghMutex);
}

void DnsResolverThread(void *p)
{
	TRACE_MSG(L"DNS resolver thread started.");
	std::unique_ptr<dns_resolver_thread> dnt(static_cast<dns_resolver_thread*>(p));
	WinMTRNet* wn = dnt->winmtr;

	const int addr = wn->GetAddr(dnt->index);
	const auto numeric = std::format(L"{}.{}.{}.{}",
		(addr >> 24) & 0xff, (addr >> 16) & 0xff,
		(addr >> 8) & 0xff,  addr & 0xff);

	struct sockaddr_in sa = {};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(addr);

	wchar_t hostname[NI_MAXHOST];
	if (GetNameInfoW((struct sockaddr*)&sa, sizeof(sa), hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD) == 0) {
		wn->SetName(dnt->index, hostname);
	} else {
		wn->SetName(dnt->index, numeric.c_str());
	}

	TRACE_MSG(L"DNS resolver thread stopped.");
	_endthread();
}
