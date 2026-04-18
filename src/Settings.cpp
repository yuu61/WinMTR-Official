//*****************************************************************************
// FILE:            Settings.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Settings.h"
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
// WinMTRSettings::InitAndLoad
//
//*****************************************************************************
BOOL WinMTRSettings::InitAndLoad(LoadedSettings& io, const LoadedSettingsFlags& flags, std::vector<CString>& outHosts)
{
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
		tmp = (DWORD)io.pingsize;
		RegSetValueExW(hConfig, L"PingSize", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasPingsize) io.pingsize = (int)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"MaxLRU", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)io.maxLRU;
		RegSetValueExW(hConfig, L"MaxLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasMaxLRU) io.maxLRU = (int)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"UseDNS", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = io.useDNS ? 1 : 0;
		RegSetValueExW(hConfig, L"UseDNS", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasUseDNS) io.useDNS = (BOOL)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hConfig, L"Interval", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)(io.interval * 1000);
		RegSetValueExW(hConfig, L"Interval", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasInterval) io.interval = (double)tmp / 1000.0;
	}
	RegCloseKey(hConfig);

	HKEY hLRU = NULL;
	if (RegCreateKeyExW(hWinMTR, L"LRU", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
						&hLRU, &disp) != ERROR_SUCCESS) {
		RegCloseKey(hWinMTR);
		return FALSE;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueExW(hLRU, L"NrLRU", 0, NULL, (BYTE*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)io.nrLRU;
		RegSetValueExW(hLRU, L"NrLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	} else {
		wchar_t hostBuf[255]{};
		io.nrLRU = (int)tmp;
		for (int i = 0; i < io.maxLRU; ++i) {
			const auto keyName = std::format(L"Host{}", i + 1);
			DWORD hostSize = sizeof(hostBuf);
			if (RegQueryValueExW(hLRU, keyName.c_str(), 0, NULL, (BYTE*)hostBuf, &hostSize) == ERROR_SUCCESS) {
				DWORD chars = hostSize / sizeof(wchar_t);
				if (chars > 0 && chars <= _countof(hostBuf))
					hostBuf[chars - 1] = L'\0';
				outHosts.push_back(CString(hostBuf));
			}
		}
	}
	RegCloseKey(hLRU);
	RegCloseKey(hWinMTR);
	return TRUE;
}


//*****************************************************************************
// WinMTRSettings::SaveOptions
//
//*****************************************************************************
void WinMTRSettings::SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval)
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


//*****************************************************************************
// WinMTRSettings::AppendLRUHost
//
//*****************************************************************************
void WinMTRSettings::AppendLRUHost(LPCWSTR host, int& nrLRU, int maxLRU)
{
	HKEY hLRU = NULL;
	if (OpenWinMTRSubKey(L"LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	if (nrLRU >= maxLRU) nrLRU = 0;
	nrLRU++;
	const auto keyName = std::format(L"Host{}", nrLRU);
	RegSetValueExW(hLRU, keyName.c_str(), 0, REG_SZ,
				   (const BYTE*)host,
				   (DWORD)((wcslen(host) + 1) * sizeof(wchar_t)));
	DWORD tmp = (DWORD)nrLRU;
	RegSetValueExW(hLRU, L"NrLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	RegCloseKey(hLRU);
}


//*****************************************************************************
// WinMTRSettings::TrimLRU
//
// Mirrors legacy off-by-one in OnOptions: deletes Host{maxLRU}..Host{nrLRU}.
//*****************************************************************************
void WinMTRSettings::TrimLRU(int maxLRU, int& nrLRU)
{
	HKEY hLRU = NULL;
	if (OpenWinMTRSubKey(L"LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	for (int i = maxLRU; i <= nrLRU; ++i) {
		RegDeleteValueW(hLRU, std::format(L"Host{}", i).c_str());
	}
	nrLRU = maxLRU;
	DWORD tmp = (DWORD)nrLRU;
	RegSetValueExW(hLRU, L"NrLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	RegCloseKey(hLRU);
}


//*****************************************************************************
// WinMTRSettings::ClearLRU
//
// Mirrors legacy ClearHistory: starts at Host0 (which never exists) and
// continues through Host{nrLRU}.
//*****************************************************************************
void WinMTRSettings::ClearLRU(int& nrLRU)
{
	HKEY hLRU = NULL;
	if (OpenWinMTRSubKey(L"LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	for (int i = 0; i <= nrLRU; ++i) {
		RegDeleteValueW(hLRU, std::format(L"Host{}", i).c_str());
	}
	nrLRU = 0;
	DWORD tmp = 0;
	RegSetValueExW(hLRU, L"NrLRU", 0, REG_DWORD, (const BYTE*)&tmp, sizeof(DWORD));
	RegCloseKey(hLRU);
}
