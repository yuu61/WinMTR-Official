//*****************************************************************************
// FILE:            TraceConfigState.h
//
// DESCRIPTION:
//   Trace-time configuration owned by the dialog. Bridges between command-line
//   flags, the Options dialog, registry persistence, and the Net worker.
//
//*****************************************************************************

#ifndef TRACECONFIGSTATE_H_
#define TRACECONFIGSTATE_H_

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

	bool hasIntervalFromCmdLine = false;
	bool hasPingsizeFromCmdLine = false;
	bool hasMaxLRUFromCmdLine   = false;
	bool hasUseDNSFromCmdLine   = false;

	// Loads persisted values. Fields set via command line keep their values.
	BOOL LoadAtInit(std::vector<CString>& outHosts);

	void SaveOptions() const;

	[[nodiscard]] TraceOptions Snapshot() const;
};

#endif // TRACECONFIGSTATE_H_
