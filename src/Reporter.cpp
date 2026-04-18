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
	char buf[255];
	CString result;

	int nh = net->GetMax();

	result  = "|------------------------------------------------------------------------------------------|\r\n";
	result += "|                                      WinMTR statistics                                   |\r\n";
	result += "|                       Host              -   %  | Sent | Recv | Best | Avrg | Wrst | Last |\r\n";
	result += "|------------------------------------------------|------|------|------|------|------|------|\r\n";

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if(strcmp(buf,"")==0) strcpy(buf,"No response from host");

		CString line;
		line.Format("|%40s - %4d | %4d | %4d | %4d | %4d | %4d | %4d |\r\n",
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		result += line;
	}

	result += "|________________________________________________|______|______|______|______|______|______|\r\n";
	result += "   ";
	result += CString((LPCSTR)IDS_STRING_SB_NAME);

	return result;
}


//*****************************************************************************
// WinMTRReporter::BuildHtmlReport
//
//*****************************************************************************
CString WinMTRReporter::BuildHtmlReport(WinMTRNet* net)
{
	char buf[255];
	CString result;

	int nh = net->GetMax();

	result  = "<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n";
	result += "<center><h2>WinMTR statistics</h2></center>\r\n";
	result += "<p align=\"center\"> <table border=\"1\" align=\"center\">\r\n";
	result += "<tr><td>Host</td> <td>%</td> <td>Sent</td> <td>Recv</td> <td>Best</td> <td>Avrg</td> <td>Wrst</td> <td>Last</td></tr>\r\n";

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if( strcmp(buf,"")==0 ) strcpy(buf,"No response from host");

		CString line;
		line.Format("<tr><td>%s</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td></tr>\r\n",
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		result += line;
	}

	result += "</table></body></html>\r\n";

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

	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, (SIZE_T)content.GetLength() + 1);
	if (clipbuffer == NULL) {
		CloseClipboard();
		return false;
	}
	char* buffer = (char*)GlobalLock(clipbuffer);
	if (buffer == NULL) {
		GlobalFree(clipbuffer);
		CloseClipboard();
		return false;
	}
	strcpy(buffer, (LPCSTR)content);
	GlobalUnlock(clipbuffer);

	SetClipboardData(CF_TEXT, clipbuffer);
	CloseClipboard();
	return true;
}


//*****************************************************************************
// WinMTRReporter::SaveToFile
//
//*****************************************************************************
bool WinMTRReporter::SaveToFile(LPCTSTR path, const CString& content)
{
	FILE* fp = fopen(path, "wt");
	if (fp == NULL) return false;
	fprintf(fp, "%s", (LPCSTR)content);
	fclose(fp);
	return true;
}
