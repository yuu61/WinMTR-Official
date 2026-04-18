//*****************************************************************************
// FILE:            Net.h
//
//
// DESCRIPTION:
//   ICMP trace engine and per-hop statistics store.
//
//*****************************************************************************

#ifndef WINMTRNET_H_
#define WINMTRNET_H_

#include "TraceConfig.h"
#include "TraceOptions.h"
#include <array>
#include <string>
#include <cwchar>


typedef ip_option_information IPINFO, *PIPINFO, FAR *LPIPINFO;

#ifdef _WIN64
typedef icmp_echo_reply32 ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#else
typedef icmp_echo_reply ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#endif

constexpr DWORD ECHO_REPLY_TIMEOUT = 5000;

struct s_nethost {
  __int32 addr = 0;              // IP as a decimal, big endian
  int xmit = 0;                  // number of PING packets sent
  int returned = 0;              // number of ICMP echo replies received
  unsigned long total = 0;       // total time
  int last = 0;                  // last time
  int best = 0;                  // best time
  int worst = 0;                 // worst time
  wchar_t name[255] = {};
};

//*****************************************************************************
// CLASS:  WinMTRNet
//
//
//*****************************************************************************

class WinMTRNet {
	typedef HANDLE (WINAPI *LPFNICMPCREATEFILE)(VOID);
	typedef BOOL  (WINAPI *LPFNICMPCLOSEHANDLE)(HANDLE);
	typedef DWORD (WINAPI *LPFNICMPSENDECHO)(HANDLE, u_long, LPVOID, WORD, LPVOID, LPVOID, DWORD, DWORD);

public:

	WinMTRNet();
	~WinMTRNet();

	// Snapshots opts into the options member, then spawns per-TTL worker
	// threads. Blocks until all workers exit.
	void	DoTrace(int address, const TraceOptions& opts);
	void	ResetHops();
	void	StopTrace();

	// Starts a worker that acquires externalMutex, resolves hostname, and
	// invokes DoTrace. On resolution failure, shows AfxMessageBox and
	// releases the mutex without tracing.
	void	BeginTraceAsync(const std::wstring& hostname, const TraceOptions& opts, HANDLE externalMutex);

	[[nodiscard]] int GetAddr(int at);
	int               GetName(int at, wchar_t *n);
	[[nodiscard]] int GetBest(int at);
	[[nodiscard]] int GetWorst(int at);
	[[nodiscard]] int GetAvg(int at);
	[[nodiscard]] int GetPercent(int at);
	[[nodiscard]] int GetLast(int at);
	[[nodiscard]] int GetReturned(int at);
	[[nodiscard]] int GetXmit(int at);
	[[nodiscard]] int GetMax();

	void	SetAddr(int at, __int32 addr);
	void	SetName(int at, const wchar_t *n);
	void	SetBest(int at, int current);
	void	SetWorst(int at, int current);
	void	SetLast(int at, int last);
	void	AddReturned(int at);
	void	AddXmit(int at);

	TraceOptions		options;
	__int32				last_remote_addr;
	bool				tracing;
	bool				initialized;
    HANDLE				hICMP;
	LPFNICMPCREATEFILE	lpfnIcmpCreateFile;
	LPFNICMPCLOSEHANDLE lpfnIcmpCloseHandle;
	LPFNICMPSENDECHO	lpfnIcmpSendEcho;
private:
	HINSTANCE			hICMP_DLL;

    std::array<s_nethost, MaxHost> host;
	HANDLE				ghMutex;
};

#endif	// ifndef WINMTRNET_H_
