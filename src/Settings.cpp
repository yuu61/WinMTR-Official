//*****************************************************************************
// FILE:            Settings.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Settings.h"
#include "TraceConfigState.h"
#include "Version.h"

namespace {

LONG OpenWinMTRRoot(REGSAM access, HKEY& outKey)
{
	HKEY hSoftware = NULL;
	DWORD disp = 0;
	LONG r = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software", 0, NULL,
							 REG_OPTION_NON_VOLATILE, access, NULL,
							 &hSoftware, &disp);
	if (r != ERROR_SUCCESS) return r;
	r = RegCreateKeyExW(hSoftware, L"WinMTR", 0, NULL,
						REG_OPTION_NON_VOLATILE, access, NULL,
						&outKey, &disp);
	RegCloseKey(hSoftware);
	return r;
}

LONG OpenWinMTRSubKey(LPCWSTR sub, REGSAM access, HKEY& outKey)
{
	HKEY hWinMTR = NULL;
	LONG r = OpenWinMTRRoot(access, hWinMTR);
	if (r != ERROR_SUCCESS) return r;
	DWORD disp = 0;
	r = RegCreateKeyExW(hWinMTR, sub, 0, NULL,
						REG_OPTION_NON_VOLATILE, access, NULL,
						&outKey, &disp);
	RegCloseKey(hWinMTR);
	return r;
}

} // namespace


//*****************************************************************************
// Settings::InitAndLoad
//
//*****************************************************************************
BOOL Settings::InitAndLoad(TraceConfigState& state, const CommandLineOverrides& overrides, std::vector<CString>& outHosts)
{
	// Apply cmdline overrides first so registry writes below (first-run path)
	// seed from the cmdline value, and registry reads correctly skip
	// overridden fields.
	if (overrides.pingsize) state.pingsize = *overrides.pingsize;
	if (overrides.interval) state.interval = *overrides.interval;
	if (overrides.maxLRU)   state.lru.SetMax(*overrides.maxLRU);
	if (overrides.useDNS)   state.useDNS   = *overrides.useDNS;

	HKEY hWinMTR = NULL;
	if (OpenWinMTRRoot(KEY_ALL_ACCESS, hWinMTR) != ERROR_SUCCESS)
		return FALSE;

	RegSetValueExW(hWinMTR, L"Version", 0, REG_SZ,
				   (const BYTE*)WINMTR_VERSION, sizeof(WINMTR_VERSION));
	RegSetValueExW(hWinMTR, L"License", 0, REG_SZ,
				   (const BYTE*)WINMTR_LICENSE, sizeof(WINMTR_LICENSE));
	RegSetValueExW(hWinMTR, L"HomePage", 0, REG_SZ,
				   (const BYTE*)WINMTR_HOMEPAGE, sizeof(WINMTR_HOMEPAGE));

	HKEY hConfig = NULL;
	DWORD disp = 0;
	if (RegCreateKeyExW(hWinMTR, L"Config", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hConfig, &disp) != ERROR_SUCCESS) {
		RegCloseKey(hWinMTR);
		return FALSE;
	}

	DWORD tmp = 0;
	DWORD sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"PingSize", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)state.pingsize;
		RegSetValueExW(hConfig, L"PingSize", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!overrides.pingsize) {
			const int v = static_cast<int>(tmp);
			state.pingsize = (v >= 0 && v <= 8184) ? v : DEFAULT_PING_SIZE;
		}
	}

	sz = sizeof(DWORD);
	int maxLRU = state.lru.Max();
	if (RegQueryValueExW(hConfig, L"MaxLRU", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)maxLRU;
		RegSetValueExW(hConfig, L"MaxLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!overrides.maxLRU) {
			const int v = static_cast<int>(tmp);
			maxLRU = (v >= 1 && v <= 10000) ? v : DEFAULT_MAX_LRU;
		}
	}
	state.lru.SetMax(maxLRU);

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"UseDNS", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = state.useDNS ? 1 : 0;
		RegSetValueExW(hConfig, L"UseDNS", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!overrides.useDNS) state.useDNS = (BOOL)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"Interval", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)(state.interval * 1000);
		RegSetValueExW(hConfig, L"Interval", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!overrides.interval) {
			const double v = static_cast<double>(tmp) / 1000.0;
			state.interval = (v >= 0.1 && v <= 60.0) ? v : DEFAULT_INTERVAL;
		}
	}
	RegCloseKey(hConfig);

	RegCloseKey(hWinMTR);
	(void)state.lru.Load(outHosts);  // history load failure is non-fatal; outHosts stays empty
	return TRUE;
}


//*****************************************************************************
// Settings::SaveOptions
//
//*****************************************************************************
void Settings::SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval)
{
	HKEY hConfig = NULL;
	if (OpenWinMTRSubKey(L"Config", KEY_ALL_ACCESS, hConfig) != ERROR_SUCCESS)
		return;

	DWORD tmp;
	tmp = (DWORD)pingsize;
	RegSetValueExW(hConfig, L"PingSize", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	tmp = (DWORD)maxLRU;
	RegSetValueExW(hConfig, L"MaxLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	tmp = useDNS ? 1u : 0u;
	RegSetValueExW(hConfig, L"UseDNS", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	tmp = (DWORD)(interval * 1000);
	RegSetValueExW(hConfig, L"Interval", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	RegCloseKey(hConfig);
}


