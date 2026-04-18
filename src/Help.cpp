// Help.cpp : implementation file
//

#include "Global.h"
#include "Help.h"
#include "afxdialogex.h"


// Help dialog

IMPLEMENT_DYNAMIC(Help, CDialog)

Help::Help(CWnd* pParent /*=NULL*/)
	: CDialog(Help::IDD, pParent)
{

}

Help::~Help()
{
}

void Help::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Help, CDialog)
	ON_BN_CLICKED(IDOK, &Help::OnBnClickedOk)
END_MESSAGE_MAP()


// Help message handlers


void Help::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
