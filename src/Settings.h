//*****************************************************************************
// FILE:            Settings.h
//
//
// DESCRIPTION:
//   Registry I/O for WinMTR. Reads/writes HKCU\Software\WinMTR\{Config,LRU}.
//
//*****************************************************************************

#ifndef WINMTRSETTINGS_H_
#define WINMTRSETTINGS_H_

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

// Indicates fields explicitly set via the command line. Such fields keep
// the caller-provided value and are not overwritten by registry data.
struct LoadedSettingsFlags {
	bool hasPingsize = false;
	bool hasInterval = false;
	bool hasMaxLRU   = false;
	bool hasUseDNS   = false;
};

//*****************************************************************************
// CLASS:  WinMTRSettings
//
//
//*****************************************************************************

class WinMTRSettings
{
public:
	// Seed io with caller's current values. On return, fields whose flag is
	// false are replaced with persisted registry values; missing registry
	// entries are seeded from io.
	static BOOL InitAndLoad(LoadedSettings& io, const LoadedSettingsFlags& flags, std::vector<CString>& outHosts);

	static void SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval);

	static void AppendLRUHost(LPCWSTR host, int& nrLRU, int maxLRU);

	static void TrimLRU(int maxLRU, int& nrLRU);

	static void ClearLRU(int& nrLRU);
};

#endif // ifndef WINMTRSETTINGS_H_
