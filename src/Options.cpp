//*****************************************************************************
// FILE:            Options.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Options.h"
#include "License.h"

#include <cwchar>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {

// Dialog text -> numeric with endptr-based validation. Leaves `out`
// untouched on parse failure so the previous value persists.
bool ParseDoubleField(LPCWSTR text, double& out)
{
	if (text == nullptr || text[0] == L'\0') return false;
	wchar_t* end = nullptr;
	errno = 0;
	const double v = std::wcstod(text, &end);
	if (end == text || errno == ERANGE) return false;
	out = v;
	return true;
}

bool ParseIntField(LPCWSTR text, int& out)
{
	if (text == nullptr || text[0] == L'\0') return false;
	wchar_t* end = nullptr;
	errno = 0;
	const long v = std::wcstol(text, &end, 10);
	if (end == text || errno == ERANGE) return false;
	out = static_cast<int>(v);
	return true;
}

} // namespace


//*****************************************************************************
// BEGIN_MESSAGE_MAP
//
// 
//*****************************************************************************
BEGIN_MESSAGE_MAP(Options, CDialog)
	ON_BN_CLICKED(ID_LICENSE, OnLicense)
END_MESSAGE_MAP()


//*****************************************************************************
// Options::Options
//
// 
//*****************************************************************************
Options::Options(CWnd* pParent) : CDialog(Options::IDD, pParent)
{
}


//*****************************************************************************
// Options::DoDataExchange
//
// 
//*****************************************************************************
void Options::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SIZE, m_editSize);
	DDX_Control(pDX, IDC_EDIT_INTERVAL, m_editInterval);
	DDX_Control(pDX, IDC_EDIT_MAX_LRU, m_editMaxLRU);
	DDX_Control(pDX, IDC_CHECK_DNS, m_checkDNS);
}


//*****************************************************************************
// Options::OnInitDialog
//
// 
//*****************************************************************************
BOOL Options::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_editInterval.SetWindowText(std::format(L"{:.1f}", interval).c_str());
	m_editSize.SetWindowText(std::format(L"{}", pingsize).c_str());
	m_editMaxLRU.SetWindowText(std::format(L"{}", maxLRU).c_str());

	m_checkDNS.SetCheck(useDNS);

	m_editInterval.SetFocus();
	return FALSE;
}


//*****************************************************************************
// Options::OnOK
//
// 
//*****************************************************************************
void Options::OnOK()
{
	wchar_t tmpstr[20];

	useDNS = m_checkDNS.GetCheck();

	m_editInterval.GetWindowText(tmpstr, 20);
	ParseDoubleField(tmpstr, interval);

	m_editSize.GetWindowText(tmpstr, 20);
	ParseIntField(tmpstr, pingsize);

	m_editMaxLRU.GetWindowText(tmpstr, 20);
	ParseIntField(tmpstr, maxLRU);

	CDialog::OnOK();
}

//*****************************************************************************
// Options::OnLicense
//
// 
//*****************************************************************************
void Options::OnLicense() 
{
	License mtrlicense;
	mtrlicense.DoModal();
}
