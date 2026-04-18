//*****************************************************************************
// FILE:            TraceConfigState.h
//
// DESCRIPTION:
//   Trace-time configuration owned by the dialog. Bridges between command-line
//   overrides, the Options dialog, registry persistence, and the Net worker.
//
//*****************************************************************************

#ifndef TRACECONFIGSTATE_H_
#define TRACECONFIGSTATE_H_

#include "CommandLineOverrides.h"
#include "TraceConfig.h"
#include "TraceOptions.h"
#include <afxwin.h>
#include <vector>

struct TraceConfigState {
	double interval = DEFAULT_INTERVAL;
	int    pingsize = DEFAULT_PING_SIZE;
	int    maxLRU   = DEFAULT_MAX_LRU;
	BOOL   useDNS   = DEFAULT_DNS;
	int    nrLRU    = 0;

	// Applies overrides and loads persisted values. Fields set via cmdline
	// win over both defaults and registry.
	BOOL LoadAtInit(const CommandLineOverrides& overrides, std::vector<CString>& outHosts);

	void SaveOptions() const;

	[[nodiscard]] TraceOptions Snapshot() const;
};

#endif // TRACECONFIGSTATE_H_
