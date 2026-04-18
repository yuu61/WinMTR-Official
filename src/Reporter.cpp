//*****************************************************************************
// FILE:            Reporter.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Reporter.h"
#include "Net.h"


//*****************************************************************************
// WinMTRReporter::BuildTextReport
//
//*****************************************************************************
CString WinMTRReporter::BuildTextReport(WinMTRNet* net)
{
	wchar_t buf[255];
	CString result;

	int nh = net->GetMax();

	result  = L"|------------------------------------------------------------------------------------------|\r\n";
	result += L"|                                      WinMTR statistics                                   |\r\n";
	result += L"|                       Host              -   %  | Sent | Recv | Best | Avrg | Wrst | Last |\r\n";
	result += L"|------------------------------------------------|------|------|------|------|------|------|\r\n";

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if(wcscmp(buf, L"")==0) wcscpy(buf, L"No response from host");

		CString line;
		line.Format(L"|%40s - %4d | %4d | %4d | %4d | %4d | %4d | %4d |\r\n",
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		result += line;
	}

	result += L"|________________________________________________|______|______|______|______|______|______|\r\n";
	result += L"   ";
	result += CString((LPCWSTR)IDS_STRING_SB_NAME);

	return result;
}


//*****************************************************************************
// WinMTRReporter::BuildHtmlReport
//
//*****************************************************************************
CString WinMTRReporter::BuildHtmlReport(WinMTRNet* net)
{
	wchar_t buf[255];
	CString result;

	int nh = net->GetMax();

	result  = L"<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n";
	result += L"<center><h2>WinMTR statistics</h2></center>\r\n";
	result += L"<p align=\"center\"> <table border=\"1\" align=\"center\">\r\n";
	result += L"<tr><td>Host</td> <td>%</td> <td>Sent</td> <td>Recv</td> <td>Best</td> <td>Avrg</td> <td>Wrst</td> <td>Last</td></tr>\r\n";

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if( wcscmp(buf, L"")==0 ) wcscpy(buf, L"No response from host");

		CString line;
		line.Format(L"<tr><td>%s</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td></tr>\r\n",
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		result += line;
	}

	result += L"</table></body></html>\r\n";

	return result;
}


//*****************************************************************************
// WinMTRReporter::CopyToClipboard
//
//*****************************************************************************
bool WinMTRReporter::CopyToClipboard(CWnd* owner, const CString& content)
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
	wcscpy(buffer, (LPCWSTR)content);
	GlobalUnlock(clipbuffer);

	SetClipboardData(CF_UNICODETEXT, clipbuffer);
	CloseClipboard();
	return true;
}


//*****************************************************************************
// WinMTRReporter::SaveToFile
//
//*****************************************************************************
bool WinMTRReporter::SaveToFile(LPCWSTR path, const CString& content)
{
	FILE* fp = _wfopen(path, L"w, ccs=UTF-8");
	if (fp == NULL) return false;
	fwprintf(fp, L"%s", (LPCWSTR)content);
	fclose(fp);
	return true;
}
