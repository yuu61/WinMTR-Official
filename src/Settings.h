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

class WinMTRDialog;

//*****************************************************************************
// CLASS:  WinMTRSettings
//
//
//*****************************************************************************

class WinMTRSettings
{
public:
	static BOOL InitAndLoad(WinMTRDialog* dlg, std::vector<CString>& outHosts);

	static void SaveOptions(int pingsize, int maxLRU, BOOL useDNS, double interval);

	static void AppendLRUHost(LPCSTR host, int& nrLRU, int maxLRU);

	static void TrimLRU(int maxLRU, int& nrLRU);

	static void ClearLRU(int& nrLRU);
};

#endif // ifndef WINMTRSETTINGS_H_
