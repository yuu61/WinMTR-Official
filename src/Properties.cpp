//*****************************************************************************
// FILE:            Properties.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Properties.h"

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
BEGIN_MESSAGE_MAP(Properties, CDialog)
END_MESSAGE_MAP()


//*****************************************************************************
// Properties::Properties
//
// 
//*****************************************************************************
Properties::Properties(CWnd* pParent) : CDialog(Properties::IDD, pParent)
{
}


//*****************************************************************************
// WinMTRroperties::DoDataExchange
//
// 
//*****************************************************************************
void Properties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PHOST, m_editHost);
	DDX_Control(pDX, IDC_EDIT_PIP, m_editIP);
	DDX_Control(pDX, IDC_EDIT_PCOMMENT, m_editComment);

	DDX_Control(pDX, IDC_EDIT_PLOSS, m_editLoss);
	DDX_Control(pDX, IDC_EDIT_PSENT, m_editSent);
	DDX_Control(pDX, IDC_EDIT_PRECV, m_editRecv);

	DDX_Control(pDX, IDC_EDIT_PLAST, m_editLast);
	DDX_Control(pDX, IDC_EDIT_PBEST, m_editBest);
	DDX_Control(pDX, IDC_EDIT_PWORST, m_editWorst);
	DDX_Control(pDX, IDC_EDIT_PAVRG, m_editAvrg);
}


//*****************************************************************************
// Properties::OnInitDialog
//
// 
//*****************************************************************************
BOOL Properties::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_editIP.SetWindowText(ip);
	m_editHost.SetWindowText(host);
	m_editComment.SetWindowText(comment);

	m_editLoss.SetWindowText(std::format(L"{}", pck_loss).c_str());
	m_editSent.SetWindowText(std::format(L"{}", pck_sent).c_str());
	m_editRecv.SetWindowText(std::format(L"{}", pck_recv).c_str());

	m_editLast.SetWindowText(std::format(L"{:.1f}", ping_last).c_str());
	m_editBest.SetWindowText(std::format(L"{:.1f}", ping_best).c_str());
	m_editWorst.SetWindowText(std::format(L"{:.1f}", ping_worst).c_str());
	m_editAvrg.SetWindowText(std::format(L"{:.1f}", ping_avrg).c_str());

	return FALSE;
}

