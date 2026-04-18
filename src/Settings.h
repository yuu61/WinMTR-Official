//*****************************************************************************
// FILE:            Settings.h
//
//
// DESCRIPTION:
//   Registry I/O for WinMTR. Reads/writes HKCU\Software\WinMTR\{Config,LRU}.
//
//*****************************************************************************

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "CommandLineOverrides.h"
#include <afxwin.h>
#include <vector>

struct TraceConfigState;

//*****************************************************************************
// CLASS:  Settings
//
//
//*****************************************************************************

class Settings
{
public:
	// Seeds state with caller's current values. Applies cmdline overrides
	// up-front, then loads the registry; registry values replace state only
	// for fields that were not overridden. Missing registry entries are seeded
	// from state (so first-run writes reflect both defaults and cmdline).
	static BOOL InitAndLoad(TraceConfigState& state, const CommandLineOverrides& overrides, std::vector<CString>& outHosts);

	static void SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval);

	static void AppendLRUHost(LPCWSTR host, int& nrLRU, int maxLRU);

	static void TrimLRU(int maxLRU, int& nrLRU);

	static void ClearLRU(int& nrLRU);
};

#endif // ifndef SETTINGS_H_
