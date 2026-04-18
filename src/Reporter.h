//*****************************************************************************
// FILE:            Reporter.h
//
//
// DESCRIPTION:
//   Builds the WinMTR statistics report (TXT/HTML) and writes it to the
//   clipboard or a file.
//
//*****************************************************************************

#ifndef WINMTRREPORTER_H_
#define WINMTRREPORTER_H_

#include <afxwin.h>

class WinMTRNet;

//*****************************************************************************
// CLASS:  WinMTRReporter
//
//
//*****************************************************************************

class WinMTRReporter
{
public:
	static CString BuildTextReport(WinMTRNet* net);
	static CString BuildHtmlReport(WinMTRNet* net);

	static bool CopyToClipboard(CWnd* owner, const CString& content);
	static bool SaveToFile(LPCWSTR path, const CString& content);
};

#endif // ifndef WINMTRREPORTER_H_
