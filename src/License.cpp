//*****************************************************************************
// FILE:            License.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "License.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//*****************************************************************************
// BEGIN_MESSAGE_MAP
//
//
//*****************************************************************************
BEGIN_MESSAGE_MAP(WinMTRLicense, CDialog)
END_MESSAGE_MAP()


//*****************************************************************************
// WinMTRLicense::WinMTRLicense
//
//
//*****************************************************************************
WinMTRLicense::WinMTRLicense(CWnd* pParent) : CDialog(WinMTRLicense::IDD, pParent)
{
}


//*****************************************************************************
// WinMTRLicense::DoDataExchange
//
//
//*****************************************************************************
void WinMTRLicense::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//*****************************************************************************
// WinMTRLicense::OnInitDialog
//
//
//*****************************************************************************
BOOL WinMTRLicense::OnInitDialog()
{
	CDialog::OnInitDialog();
	return FALSE;
}
