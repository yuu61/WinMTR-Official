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
BEGIN_MESSAGE_MAP(License, CDialog)
END_MESSAGE_MAP()


//*****************************************************************************
// License::License
//
//
//*****************************************************************************
License::License(CWnd* pParent) : CDialog(License::IDD, pParent)
{
}


//*****************************************************************************
// License::DoDataExchange
//
//
//*****************************************************************************
void License::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//*****************************************************************************
// License::OnInitDialog
//
//
//*****************************************************************************
BOOL License::OnInitDialog()
{
	CDialog::OnInitDialog();
	return FALSE;
}
