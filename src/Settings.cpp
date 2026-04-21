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

constexpr int    kMinInterval      = 0;        // raw ms stored in registry
constexpr double kMinIntervalSec   = 0.1;
constexpr double kMaxIntervalSec   = 60.0;
constexpr double kIntervalMsScale  = 1000.0;
constexpr int    kMinPingSize      = 0;
constexpr int    kMaxPingSize      = 8184;
constexpr int    kMinMaxLRU        = 1;
constexpr int    kMaxMaxLRU        = 10000;
constexpr DWORD  kRegBoolTrue      = 1;
constexpr DWORD  kRegBoolFalse     = 0;

LONG OpenWinMTRRoot(REGSAM access, HKEY& outKey)
{
	HKEY  hSoftware = nullptr;
	DWORD disp      = 0;
	LONG  r         = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software", 0, nullptr,
	                                  REG_OPTION_NON_VOLATILE, access, nullptr,
	                                  &hSoftware, &disp);
	if (r != ERROR_SUCCESS) {
		return r;
	}
	r = RegCreateKeyExW(hSoftware, L"WinMTR", 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, access, nullptr,
	                    &outKey, &disp);
	RegCloseKey(hSoftware);
	return r;
}

LONG OpenWinMTRSubKey(LPCWSTR sub, REGSAM access, HKEY& outKey)
{
	HKEY hWinMTR = nullptr;
	LONG r       = OpenWinMTRRoot(access, hWinMTR);
	if (r != ERROR_SUCCESS) {
		return r;
	}
	DWORD disp = 0;
	r = RegCreateKeyExW(hWinMTR, sub, 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, access, nullptr,
	                    &outKey, &disp);
	RegCloseKey(hWinMTR);
	return r;
}

void WriteDwordValue(HKEY hKey, LPCWSTR name, DWORD value)
{
	RegSetValueExW(hKey, name, 0, REG_DWORD,
	               reinterpret_cast<const BYTE*>(&value), sizeof(DWORD));
}

// Reads a DWORD registry value. Returns true on success.
bool ReadDwordValue(HKEY hKey, LPCWSTR name, DWORD& out)
{
	DWORD sz = sizeof(DWORD);
	return RegQueryValueExW(hKey, name, nullptr, nullptr,
	                        reinterpret_cast<BYTE*>(&out), &sz) == ERROR_SUCCESS;
}

// Loads or seeds a clamped int setting. When the key is missing the current
// `current` value is written back; when present it replaces `current` unless
// the override flag says otherwise.
void LoadOrSeedInt(HKEY hKey, LPCWSTR name, int& current, int minVal, int maxVal,
                   int fallback, bool overridden)
{
	DWORD tmp = 0;
	if (!ReadDwordValue(hKey, name, tmp)) {
		WriteDwordValue(hKey, name, static_cast<DWORD>(current));
		return;
	}
	if (overridden) {
		return;
	}
	const int v = static_cast<int>(tmp);
	current = (v >= minVal && v <= maxVal) ? v : fallback;
}

void WriteStringValue(HKEY hKey, LPCWSTR name, LPCWSTR data, DWORD dataSizeBytes)
{
	RegSetValueExW(hKey, name, 0, REG_SZ,
	               reinterpret_cast<const BYTE*>(data), dataSizeBytes);
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
	if (overrides.pingsize) {
		state.pingsize = *overrides.pingsize;
	}
	if (overrides.interval) {
		state.interval = *overrides.interval;
	}
	if (overrides.maxLRU) {
		state.lru.SetMax(*overrides.maxLRU);
	}
	if (overrides.useDNS) {
		state.useDNS = *overrides.useDNS;
	}

	HKEY hWinMTR = nullptr;
	if (OpenWinMTRRoot(KEY_ALL_ACCESS, hWinMTR) != ERROR_SUCCESS) {
		return FALSE;
	}

	WriteStringValue(hWinMTR, L"Version",  WINMTR_VERSION,  sizeof(WINMTR_VERSION));
	WriteStringValue(hWinMTR, L"License",  WINMTR_LICENSE,  sizeof(WINMTR_LICENSE));
	WriteStringValue(hWinMTR, L"HomePage", WINMTR_HOMEPAGE, sizeof(WINMTR_HOMEPAGE));

	HKEY  hConfig = nullptr;
	DWORD disp    = 0;
	if (RegCreateKeyExW(hWinMTR, L"Config", 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr,
	                    &hConfig, &disp) != ERROR_SUCCESS) {
		RegCloseKey(hWinMTR);
		return FALSE;
	}

	LoadOrSeedInt(hConfig, L"PingSize", state.pingsize,
	              kMinPingSize, kMaxPingSize, DEFAULT_PING_SIZE,
	              overrides.pingsize.has_value());

	int maxLRU = state.lru.Max();
	LoadOrSeedInt(hConfig, L"MaxLRU", maxLRU,
	              kMinMaxLRU, kMaxMaxLRU, DEFAULT_MAX_LRU,
	              overrides.maxLRU.has_value());
	state.lru.SetMax(maxLRU);

	DWORD tmp = 0;
	if (!ReadDwordValue(hConfig, L"UseDNS", tmp)) {
		WriteDwordValue(hConfig, L"UseDNS", state.useDNS ? kRegBoolTrue : kRegBoolFalse);
	} else if (!overrides.useDNS) {
		state.useDNS = static_cast<BOOL>(tmp);
	}

	if (!ReadDwordValue(hConfig, L"Interval", tmp)) {
		WriteDwordValue(hConfig, L"Interval",
		                static_cast<DWORD>(state.interval * kIntervalMsScale));
	} else if (!overrides.interval) {
		const double v = static_cast<double>(tmp) / kIntervalMsScale;
		state.interval = (v >= kMinIntervalSec && v <= kMaxIntervalSec) ? v : DEFAULT_INTERVAL;
	}
	RegCloseKey(hConfig);

	RegCloseKey(hWinMTR);
	(void)state.lru.Load(outHosts); // history load failure is non-fatal; outHosts stays empty
	(void)kMinInterval; // reserved for future symmetry with other ranges
	return TRUE;
}


//*****************************************************************************
// Settings::SaveOptions
//
//*****************************************************************************
void Settings::SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval)
{
	HKEY hConfig = nullptr;
	if (OpenWinMTRSubKey(L"Config", KEY_ALL_ACCESS, hConfig) != ERROR_SUCCESS) {
		return;
	}

	WriteDwordValue(hConfig, L"PingSize", static_cast<DWORD>(pingsize));
	WriteDwordValue(hConfig, L"MaxLRU",   static_cast<DWORD>(maxLRU));
	WriteDwordValue(hConfig, L"UseDNS",   useDNS ? kRegBoolTrue : kRegBoolFalse);
	WriteDwordValue(hConfig, L"Interval", static_cast<DWORD>(interval * kIntervalMsScale));
	RegCloseKey(hConfig);
}
