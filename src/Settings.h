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

// Values exchanged with the registry on load.
struct LoadedSettings {
	int    pingsize = 0;
	double interval = 0.0;
	int    maxLRU   = 0;
	BOOL   useDNS   = FALSE;
	int    nrLRU    = 0;
};

//*****************************************************************************
// CLASS:  Settings
//
//
//*****************************************************************************

class Settings
{
public:
	// Seed io with caller's current values. Applies cmdline overrides up-front,
	// then loads the registry; registry values replace io only for fields that
	// were not overridden on the command line. Missing registry entries are
	// seeded from io (so first-run writes reflect both defaults and cmdline).
	static BOOL InitAndLoad(LoadedSettings& io, const CommandLineOverrides& overrides, std::vector<CString>& outHosts);

	static void SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval);

	static void AppendLRUHost(LPCWSTR host, int& nrLRU, int maxLRU);

	static void TrimLRU(int maxLRU, int& nrLRU);

	static void ClearLRU(int& nrLRU);
};

#endif // ifndef SETTINGS_H_
