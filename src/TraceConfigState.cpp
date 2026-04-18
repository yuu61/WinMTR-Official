//*****************************************************************************
// FILE:            TraceConfigState.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceConfigState.h"
#include "Settings.h"

BOOL TraceConfigState::LoadAtInit(const CommandLineOverrides& overrides, std::vector<CString>& outHosts)
{
	return Settings::InitAndLoad(*this, overrides, outHosts);
}

void TraceConfigState::SaveOptions() const
{
	Settings::SaveOptions(pingsize, lru.Max(), useDNS, interval);
}

TraceOptions TraceConfigState::Snapshot() const
{
	TraceOptions opts{};
	opts.pingsize = pingsize;
	opts.interval = interval;
	opts.useDNS   = useDNS;
	return opts;
}
