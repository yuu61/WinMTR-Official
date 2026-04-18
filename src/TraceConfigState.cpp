//*****************************************************************************
// FILE:            TraceConfigState.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceConfigState.h"
#include "Settings.h"

BOOL TraceConfigState::LoadAtInit(const CommandLineOverrides& overrides, std::vector<CString>& outHosts)
{
	LoadedSettings s{};
	s.pingsize = pingsize;
	s.interval = interval;
	s.maxLRU   = maxLRU;
	s.useDNS   = useDNS;
	s.nrLRU    = nrLRU;

	if (!WinMTRSettings::InitAndLoad(s, overrides, outHosts))
		return FALSE;

	pingsize = s.pingsize;
	interval = s.interval;
	maxLRU   = s.maxLRU;
	useDNS   = s.useDNS;
	nrLRU    = s.nrLRU;
	return TRUE;
}

void TraceConfigState::SaveOptions() const
{
	WinMTRSettings::SaveOptions(pingsize, maxLRU, useDNS, interval);
}

TraceOptions TraceConfigState::Snapshot() const
{
	TraceOptions opts{};
	opts.pingsize = pingsize;
	opts.interval = interval;
	opts.useDNS   = useDNS;
	return opts;
}
