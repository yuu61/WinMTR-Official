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
	LONG r = RegCreateKeyEx(HKEY_CURRENT_USER, "Software", 0, NULL,
							REG_OPTION_NON_VOLATILE, access, NULL,
							&hSoftware, &disp);
	if (r != ERROR_SUCCESS) return r;
	r = RegCreateKeyEx(hSoftware, "WinMTR", 0, NULL,
					   REG_OPTION_NON_VOLATILE, access, NULL,
					   &outKey, &disp);
	RegCloseKey(hSoftware);
	return r;
}

LONG OpenWinMTRSubKey(LPCSTR sub, REGSAM access, HKEY& outKey)
{
	HKEY hWinMTR = NULL;
	LONG r = OpenWinMTRRoot(access, hWinMTR);
	if (r != ERROR_SUCCESS) return r;
	DWORD disp = 0;
	r = RegCreateKeyEx(hWinMTR, sub, 0, NULL,
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

	RegSetValueEx(hWinMTR, "Version", 0, REG_SZ,
				  (const unsigned char*)WINMTR_VERSION, sizeof(WINMTR_VERSION));
	RegSetValueEx(hWinMTR, "License", 0, REG_SZ,
				  (const unsigned char*)WINMTR_LICENSE, sizeof(WINMTR_LICENSE));
	RegSetValueEx(hWinMTR, "HomePage", 0, REG_SZ,
				  (const unsigned char*)WINMTR_HOMEPAGE, sizeof(WINMTR_HOMEPAGE));

	HKEY hConfig = NULL;
	DWORD disp = 0;
	if (RegCreateKeyEx(hWinMTR, "Config", 0, NULL,
					   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
					   &hConfig, &disp) != ERROR_SUCCESS) {
		RegCloseKey(hWinMTR);
		return FALSE;
	}

	DWORD tmp = 0;
	DWORD sz = sizeof(DWORD);
	if (RegQueryValueEx(hConfig, "PingSize", 0, NULL, (unsigned char*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)io.pingsize;
		RegSetValueEx(hConfig, "PingSize", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasPingsize) io.pingsize = (int)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueEx(hConfig, "MaxLRU", 0, NULL, (unsigned char*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)io.maxLRU;
		RegSetValueEx(hConfig, "MaxLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasMaxLRU) io.maxLRU = (int)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueEx(hConfig, "UseDNS", 0, NULL, (unsigned char*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = io.useDNS ? 1 : 0;
		RegSetValueEx(hConfig, "UseDNS", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasUseDNS) io.useDNS = (BOOL)tmp;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueEx(hConfig, "Interval", 0, NULL, (unsigned char*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)(io.interval * 1000);
		RegSetValueEx(hConfig, "Interval", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	} else {
		if (!flags.hasInterval) io.interval = (double)tmp / 1000.0;
	}
	RegCloseKey(hConfig);

	HKEY hLRU = NULL;
	if (RegCreateKeyEx(hWinMTR, "LRU", 0, NULL,
					   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
					   &hLRU, &disp) != ERROR_SUCCESS) {
		RegCloseKey(hWinMTR);
		return FALSE;
	}

	sz = sizeof(DWORD);
	if (RegQueryValueEx(hLRU, "NrLRU", 0, NULL, (unsigned char*)&tmp, &sz) != ERROR_SUCCESS) {
		tmp = (DWORD)io.nrLRU;
		RegSetValueEx(hLRU, "NrLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	} else {
		char keyName[20];
		unsigned char hostBuf[255];
		io.nrLRU = (int)tmp;
		for (int i = 0; i < io.maxLRU; ++i) {
			sprintf(keyName, "Host%d", i + 1);
			DWORD hostSize = 0;
			if (RegQueryValueEx(hLRU, keyName, 0, NULL, NULL, &hostSize) == ERROR_SUCCESS) {
				RegQueryValueEx(hLRU, keyName, 0, NULL, hostBuf, &hostSize);
				hostBuf[hostSize] = '\0';
				outHosts.push_back(CString((LPCSTR)hostBuf));
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
	if (OpenWinMTRSubKey("Config", KEY_ALL_ACCESS, hConfig) != ERROR_SUCCESS)
		return;

	DWORD tmp;
	tmp = (DWORD)pingsize;
	RegSetValueEx(hConfig, "PingSize", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	tmp = (DWORD)maxLRU;
	RegSetValueEx(hConfig, "MaxLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	tmp = useDNS ? 1u : 0u;
	RegSetValueEx(hConfig, "UseDNS", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	tmp = (DWORD)(interval * 1000);
	RegSetValueEx(hConfig, "Interval", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	RegCloseKey(hConfig);
}


//*****************************************************************************
// WinMTRSettings::AppendLRUHost
//
//*****************************************************************************
void WinMTRSettings::AppendLRUHost(LPCSTR host, int& nrLRU, int maxLRU)
{
	HKEY hLRU = NULL;
	if (OpenWinMTRSubKey("LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	if (nrLRU >= maxLRU) nrLRU = 0;
	nrLRU++;
	char keyName[20];
	sprintf(keyName, "Host%d", nrLRU);
	RegSetValueEx(hLRU, keyName, 0, REG_SZ,
				  (const unsigned char*)host,
				  (DWORD)(strlen(host) + 1));
	DWORD tmp = (DWORD)nrLRU;
	RegSetValueEx(hLRU, "NrLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
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
	if (OpenWinMTRSubKey("LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	char keyName[20];
	for (int i = maxLRU; i <= nrLRU; ++i) {
		sprintf(keyName, "Host%d", i);
		RegDeleteValue(hLRU, keyName);
	}
	nrLRU = maxLRU;
	DWORD tmp = (DWORD)nrLRU;
	RegSetValueEx(hLRU, "NrLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
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
	if (OpenWinMTRSubKey("LRU", KEY_ALL_ACCESS, hLRU) != ERROR_SUCCESS)
		return;

	char keyName[20];
	for (int i = 0; i <= nrLRU; ++i) {
		sprintf(keyName, "Host%d", i);
		RegDeleteValue(hLRU, keyName);
	}
	nrLRU = 0;
	DWORD tmp = 0;
	RegSetValueEx(hLRU, "NrLRU", 0, REG_DWORD, (const unsigned char*)&tmp, sizeof(DWORD));
	RegCloseKey(hLRU);
}
