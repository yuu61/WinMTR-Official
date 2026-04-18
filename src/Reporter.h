//*****************************************************************************
// FILE:            Reporter.h
//
//
// DESCRIPTION:
//   Builds the WinMTR statistics report (TXT/HTML) and writes it to the
//   clipboard or a file.
//
//*****************************************************************************

#ifndef REPORTER_H_
#define REPORTER_H_

#include <afxwin.h>

class HopStatistics;

//*****************************************************************************
// CLASS:  Reporter
//
//
//*****************************************************************************

class Reporter
{
public:
	static CString BuildTextReport(const HopStatistics& stats);
	static CString BuildHtmlReport(const HopStatistics& stats);

	static bool CopyToClipboard(CWnd* owner, const CString& content);
	static bool SaveToFile(LPCWSTR path, const CString& content);
};

#endif // ifndef REPORTER_H_
