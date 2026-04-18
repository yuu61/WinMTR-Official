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
	char buf[255], t_buf[1000], f_buf[255*100];

	int nh = net->GetMax();

	strcpy(f_buf,  "|------------------------------------------------------------------------------------------|\r\n");
	sprintf(t_buf, "|                                      WinMTR statistics                                   |\r\n");
	strcat(f_buf, t_buf);
	sprintf(t_buf, "|                       Host              -   %%  | Sent | Recv | Best | Avrg | Wrst | Last |\r\n" );
	strcat(f_buf, t_buf);
	sprintf(t_buf, "|------------------------------------------------|------|------|------|------|------|------|\r\n" );
	strcat(f_buf, t_buf);

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if(strcmp(buf,"")==0) strcpy(buf,"No response from host");

		sprintf(t_buf, "|%40s - %4d | %4d | %4d | %4d | %4d | %4d | %4d |\r\n" ,
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		strcat(f_buf, t_buf);
	}

	sprintf(t_buf, "|________________________________________________|______|______|______|______|______|______|\r\n" );
	strcat(f_buf, t_buf);

	CString cs_tmp((LPCSTR)IDS_STRING_SB_NAME);
	strcat(f_buf, "   ");
	strcat(f_buf, (LPCTSTR)cs_tmp);

	return CString(f_buf);
}


//*****************************************************************************
// WinMTRReporter::BuildHtmlReport
//
//*****************************************************************************
CString WinMTRReporter::BuildHtmlReport(WinMTRNet* net)
{
	char buf[255], t_buf[1000], f_buf[255*100];

	int nh = net->GetMax();

	strcpy(f_buf, "<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n");
	sprintf(t_buf, "<center><h2>WinMTR statistics</h2></center>\r\n");
	strcat(f_buf, t_buf);

	sprintf(t_buf, "<p align=\"center\"> <table border=\"1\" align=\"center\">\r\n" );
	strcat(f_buf, t_buf);

	sprintf(t_buf, "<tr><td>Host</td> <td>%%</td> <td>Sent</td> <td>Recv</td> <td>Best</td> <td>Avrg</td> <td>Wrst</td> <td>Last</td></tr>\r\n" );
	strcat(f_buf, t_buf);

	for(int i=0;i <nh ; i++) {
		net->GetName(i, buf);
		if( strcmp(buf,"")==0 ) strcpy(buf,"No response from host");

		sprintf(t_buf, "<tr><td>%s</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td></tr>\r\n" ,
					buf, net->GetPercent(i),
					net->GetXmit(i), net->GetReturned(i), net->GetBest(i),
					net->GetAvg(i), net->GetWorst(i), net->GetLast(i));
		strcat(f_buf, t_buf);
	}

	sprintf(t_buf, "</table></body></html>\r\n");
	strcat(f_buf, t_buf);

	return CString(f_buf);
}


//*****************************************************************************
// WinMTRReporter::CopyToClipboard
//
//*****************************************************************************
bool WinMTRReporter::CopyToClipboard(CWnd* owner, const CString& content)
{
	owner->OpenClipboard();
	EmptyClipboard();

	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, (SIZE_T)content.GetLength() + 1);
	char* buffer = (char*)GlobalLock(clipbuffer);
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
