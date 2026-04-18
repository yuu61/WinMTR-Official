//*****************************************************************************
// FILE:            Reporter.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Reporter.h"
#include "HopStatistics.h"


//*****************************************************************************
// Reporter::BuildTextReport
//
//*****************************************************************************
CString Reporter::BuildTextReport(const HopStatistics& stats)
{
	wchar_t buf[255];
	CString result;

	int nh = stats.GetMax();

	result  = L"|------------------------------------------------------------------------------------------|\r\n";
	result += L"|                                      WinMTR statistics                                   |\r\n";
	result += L"|                       Host              -   %  | Sent | Recv | Best | Avrg | Wrst | Last |\r\n";
	result += L"|------------------------------------------------|------|------|------|------|------|------|\r\n";

	for(int i=0;i <nh ; i++) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') wcscpy_s(buf, L"No response from host");

		CString line;
		line.Format(L"|%40s - %4d | %4d | %4d | %4d | %4d | %4d | %4d |\r\n",
					buf, stats.GetPercent(i),
					stats.GetXmit(i), stats.GetReturned(i), stats.GetBest(i),
					stats.GetAvg(i), stats.GetWorst(i), stats.GetLast(i));
		result += line;
	}

	result += L"|________________________________________________|______|______|______|______|______|______|\r\n";
	result += L"   ";
	result += CString((LPCWSTR)IDS_STRING_SB_NAME);

	return result;
}


//*****************************************************************************
// Reporter::BuildHtmlReport
//
//*****************************************************************************
CString Reporter::BuildHtmlReport(const HopStatistics& stats)
{
	wchar_t buf[255];
	CString result;

	int nh = stats.GetMax();

	result  = L"<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n";
	result += L"<center><h2>WinMTR statistics</h2></center>\r\n";
	result += L"<p align=\"center\"> <table border=\"1\" align=\"center\">\r\n";
	result += L"<tr><td>Host</td> <td>%</td> <td>Sent</td> <td>Recv</td> <td>Best</td> <td>Avrg</td> <td>Wrst</td> <td>Last</td></tr>\r\n";

	for(int i=0;i <nh ; i++) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') wcscpy_s(buf, L"No response from host");

		CString line;
		line.Format(L"<tr><td>%s</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td></tr>\r\n",
					buf, stats.GetPercent(i),
					stats.GetXmit(i), stats.GetReturned(i), stats.GetBest(i),
					stats.GetAvg(i), stats.GetWorst(i), stats.GetLast(i));
		result += line;
	}

	result += L"</table></body></html>\r\n";

	return result;
}


//*****************************************************************************
// Reporter::CopyToClipboard
//
//*****************************************************************************
bool Reporter::CopyToClipboard(CWnd* owner, const CString& content)
{
	if (!owner->OpenClipboard())
		return false;
	EmptyClipboard();

	SIZE_T bytes = ((SIZE_T)content.GetLength() + 1) * sizeof(wchar_t);
	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, bytes);
	if (clipbuffer == NULL) {
		CloseClipboard();
		return false;
	}
	wchar_t* buffer = (wchar_t*)GlobalLock(clipbuffer);
	if (buffer == NULL) {
		GlobalFree(clipbuffer);
		CloseClipboard();
		return false;
	}
	const SIZE_T chars = bytes / sizeof(wchar_t);
	wcscpy_s(buffer, chars, (LPCWSTR)content);
	GlobalUnlock(clipbuffer);

	SetClipboardData(CF_UNICODETEXT, clipbuffer);
	CloseClipboard();
	return true;
}


//*****************************************************************************
// Reporter::SaveToFile
//
//*****************************************************************************
bool Reporter::SaveToFile(LPCWSTR path, const CString& content)
{
	FILE* fp = _wfopen(path, L"w, ccs=UTF-8");
	if (fp == NULL) return false;
	fwprintf(fp, L"%s", (LPCWSTR)content);
	fclose(fp);
	return true;
}
