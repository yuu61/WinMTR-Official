//*****************************************************************************
// FILE:            Dialog.cpp
//*****************************************************************************

#include "Global.h"
#include "Dialog.h"
#include "Options.h"
#include "Properties.h"
#include "Net.h"
#include "Settings.h"
#include "Reporter.h"
#include "HostResolver.h"
#include "Version.h"
#include "DialogLayout.h"
#include "HostComboModel.h"
#include "TraceListView.h"
#include "TraceSessionController.h"
#include <vector>
#include "afxlinkctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(WinMTRDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_RESTART, OnRestart)
	ON_BN_CLICKED(ID_OPTIONS, OnOptions)
	ON_BN_CLICKED(ID_CTTC, OnCTTC)
	ON_BN_CLICKED(ID_CHTC, OnCHTC)
	ON_BN_CLICKED(ID_EXPT, OnEXPT)
	ON_BN_CLICKED(ID_EXPH, OnEXPH)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MTR, OnDblclkList)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, &WinMTRDialog::OnCbnSelchangeComboHost)
	ON_CBN_SELENDOK(IDC_COMBO_HOST, &WinMTRDialog::OnCbnSelendokComboHost)
	ON_CBN_CLOSEUP(IDC_COMBO_HOST, &WinMTRDialog::OnCbnCloseupComboHost)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &WinMTRDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


WinMTRDialog::WinMTRDialog(CWnd* pParent)
	: CDialog(WinMTRDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_autostart = 0;
	controller = std::make_unique<TraceSessionController>(this);
}

WinMTRDialog::~WinMTRDialog()
{
}


void WinMTRDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_OPTIONS, m_buttonOptions);
	DDX_Control(pDX, IDCANCEL, m_buttonExit);
	DDX_Control(pDX, ID_RESTART, m_buttonStart);
	DDX_Control(pDX, IDC_COMBO_HOST, m_comboHost);
	DDX_Control(pDX, IDC_LIST_MTR, m_listMTR);
	DDX_Control(pDX, IDC_STATICS, m_staticS);
	DDX_Control(pDX, IDC_STATICJ, m_staticJ);
	DDX_Control(pDX, ID_EXPH, m_buttonExpH);
	DDX_Control(pDX, ID_EXPT, m_buttonExpT);
}


BOOL WinMTRDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

#ifndef _WIN64
	wchar_t caption[] = L"WinMTR v" WINMTR_VERSION L" 32 bit by Appnor MSP - www.winmtr.net";
#else
	wchar_t caption[] = L"WinMTR v" WINMTR_VERSION L" 64 bit by Appnor MSP - www.winmtr.net";
#endif

	SetTimer(1, WINMTR_DIALOG_TIMER, NULL);
	SetWindowText(caption);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	if (!statusBar.Create(this))
		AfxMessageBox(L"Error creating status bar");
	statusBar.GetStatusBarCtrl().SetMinHeight(23);

	UINT sbi[1]{};
	sbi[0] = IDS_STRING_SB_NAME;
	statusBar.SetIndicators(sbi, 1);
	statusBar.SetPaneInfo(0, statusBar.GetItemID(0), SBPS_STRETCH, NULL);

	{
		m_appnorLink.reset(new CMFCLinkCtrl);
		if (!m_appnorLink->Create(L"www.appnor.com", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			CRect(0, 0, 0, 0), &statusBar, 1234)) {
			TRACE(L"Failed to create button control.\n");
			return FALSE;
		}

		m_appnorLink->SetURL(L"http://www.appnor.com/?utm_source=winmtr&utm_medium=desktop&utm_campaign=software");

		if (!statusBar.AddPane(1234, 1)) {
			AfxMessageBox(L"Pane index out of range\nor pane with same ID already exists in the status bar", MB_ICONERROR);
			return FALSE;
		}

		statusBar.SetPaneWidth(statusBar.CommandToIndex(1234), 100);
		statusBar.AddPaneControl(m_appnorLink.get(), 1234, FALSE);
	}

	WinMTRTraceListView::InitColumns(m_listMTR);

	m_comboHost.SetFocus();

	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
	               0, reposQuery, rcClientNow);

	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
	                rcClientNow.top - rcClientStart.top);

	CRect rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	{
		std::vector<CString> lruHosts;
		if (config.LoadAtInit(lruHosts))
			WinMTRHostComboModel::Populate(m_comboHost, lruHosts);
	}

	if (m_autostart) {
		m_comboHost.SetWindowText(msz_defaulthostname);
		OnRestart();
	}

	return FALSE;
}


void WinMTRDialog::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	WinMTRDialogLayout::ClampMinimum(pRect);
}


void WinMTRDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	WinMTRDialogLayout::ControlRefs refs{
		m_staticS, m_staticJ, m_buttonExit, m_buttonExpH, m_buttonExpT, m_listMTR
	};
	WinMTRDialogLayout::ApplyClientSize(*this, refs);
}


void WinMTRDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width()  - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


HCURSOR WinMTRDialog::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void WinMTRDialog::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (!controller->IsTracing())
		return;

	POSITION pos = m_listMTR.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;

	int nItem = m_listMTR.GetNextSelectedItem(pos);
	WinMTRProperties wmtrprop;
	WinMTRNet& net = controller->Net();

	if (net.GetAddr(nItem) == 0) {
		wmtrprop.host[0] = L'\0';
		wmtrprop.ip[0]   = L'\0';
		net.GetName(nItem, wmtrprop.comment);

		wmtrprop.pck_loss  = wmtrprop.pck_sent  = wmtrprop.pck_recv  = 0;
		wmtrprop.ping_avrg = wmtrprop.ping_last = 0.0;
		wmtrprop.ping_best = wmtrprop.ping_worst = 0.0;
	} else {
		net.GetName(nItem, wmtrprop.host);
		int addr = net.GetAddr(nItem);
		swprintf(wmtrprop.ip, _countof(wmtrprop.ip), L"%d.%d.%d.%d",
			(addr >> 24) & 0xff,
			(addr >> 16) & 0xff,
			(addr >>  8) & 0xff,
			addr         & 0xff);
		wcscpy_s(wmtrprop.comment, L"Host alive.");

		wmtrprop.ping_avrg  = (float)net.GetAvg(nItem);
		wmtrprop.ping_last  = (float)net.GetLast(nItem);
		wmtrprop.ping_best  = (float)net.GetBest(nItem);
		wmtrprop.ping_worst = (float)net.GetWorst(nItem);

		wmtrprop.pck_loss = net.GetPercent(nItem);
		wmtrprop.pck_recv = net.GetReturned(nItem);
		wmtrprop.pck_sent = net.GetXmit(nItem);
	}

	wmtrprop.DoModal();
}


void WinMTRDialog::SetHostName(const wchar_t* host)
{
	m_autostart = 1;
	wcsncpy_s(msz_defaulthostname, host, _TRUNCATE);
}


void WinMTRDialog::OnRestart()
{
	if (WinMTRHostComboModel::IsSentinelSelected(m_comboHost)) {
		ClearHistory();
		return;
	}

	if (controller->CurrentState() == TraceSessionController::IDLE) {
		CString sHost;
		m_comboHost.GetWindowText(sHost);
		sHost.TrimLeft();
		sHost.TrimLeft();

		if (sHost.IsEmpty()) {
			AfxMessageBox(L"No host specified!");
			m_comboHost.SetFocus();
			return;
		}
		m_listMTR.DeleteAllItems();

		if (!WinMTRHostResolver::LooksNumeric((LPCWSTR)sHost)) {
			statusBar.SetPaneText(0,
				std::format(L"Resolving host {}...", (LPCWSTR)sHost).c_str());
		}

		CString err;
		if (!WinMTRHostResolver::Validate((LPCWSTR)sHost, err)) {
			statusBar.SetPaneText(0, CString((LPCWSTR)IDS_STRING_SB_NAME));
			AfxMessageBox(err);
			return;
		}

		if (WinMTRHostComboModel::AppendBeforeSentinel(m_comboHost, sHost))
			WinMTRSettings::AppendLRUHost((LPCWSTR)sHost, config.nrLRU, config.maxLRU);

		controller->RequestStart((LPCWSTR)sHost, config.Snapshot());
	} else {
		controller->RequestStop();
	}
}


void WinMTRDialog::OnOptions()
{
	WinMTROptions optDlg;

	optDlg.SetPingSize(config.pingsize);
	optDlg.SetInterval(config.interval);
	optDlg.SetMaxLRU(config.maxLRU);
	optDlg.SetUseDNS(config.useDNS);

	if (IDOK == optDlg.DoModal()) {
		config.pingsize = optDlg.GetPingSize();
		config.interval = optDlg.GetInterval();
		config.maxLRU   = optDlg.GetMaxLRU();
		config.useDNS   = optDlg.GetUseDNS();

		config.SaveOptions();
		if (config.maxLRU < config.nrLRU)
			WinMTRSettings::TrimLRU(config.maxLRU, config.nrLRU);
	}
}


void WinMTRDialog::OnCTTC()
{
	WinMTRReporter::CopyToClipboard(this, WinMTRReporter::BuildTextReport(&controller->Net()));
}


void WinMTRDialog::OnCHTC()
{
	WinMTRReporter::CopyToClipboard(this, WinMTRReporter::BuildHtmlReport(&controller->Net()));
}


void WinMTRDialog::OnEXPT()
{
	wchar_t BASED_CODE szFilter[] = L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L"TXT", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, szFilter, this);
	if (dlg.DoModal() == IDOK)
		WinMTRReporter::SaveToFile(dlg.GetPathName(), WinMTRReporter::BuildTextReport(&controller->Net()));
}


void WinMTRDialog::OnEXPH()
{
	wchar_t BASED_CODE szFilter[] = L"HTML Files (*.htm, *.html)|*.htm;*.html|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L"HTML", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, szFilter, this);
	if (dlg.DoModal() == IDOK)
		WinMTRReporter::SaveToFile(dlg.GetPathName(), WinMTRReporter::BuildHtmlReport(&controller->Net()));
}


void WinMTRDialog::OnCancel()
{
}


void WinMTRDialog::OnCbnSelchangeComboHost()
{
}


void WinMTRDialog::ClearHistory()
{
	WinMTRSettings::ClearLRU(config.nrLRU);
	WinMTRHostComboModel::ClearAndResetSentinel(m_comboHost);
}


void WinMTRDialog::OnCbnSelendokComboHost()
{
}


void WinMTRDialog::OnCbnCloseupComboHost()
{
	if (WinMTRHostComboModel::IsSentinelSelected(m_comboHost))
		ClearHistory();
}


void WinMTRDialog::OnTimer(UINT_PTR nIDEvent)
{
	controller->Tick();
	CDialog::OnTimer(nIDEvent);
}


void WinMTRDialog::OnClose()
{
	controller->RequestExit();
}


void WinMTRDialog::OnBnClickedCancel()
{
	controller->RequestExit();
}


void WinMTRDialog::SetStartEnabled(bool enabled)
{
	m_buttonStart.EnableWindow(enabled ? TRUE : FALSE);
}


void WinMTRDialog::SetStartText(LPCWSTR text)
{
	m_buttonStart.SetWindowText(text);
}


void WinMTRDialog::SetHostComboEnabled(bool enabled)
{
	m_comboHost.EnableWindow(enabled ? TRUE : FALSE);
}


void WinMTRDialog::SetOptionsEnabled(bool enabled)
{
	m_buttonOptions.EnableWindow(enabled ? TRUE : FALSE);
}


void WinMTRDialog::SetStatus(LPCWSTR text)
{
	statusBar.SetPaneText(0, text);
}


void WinMTRDialog::RefreshList()
{
	WinMTRTraceListView::Refresh(m_listMTR, controller->Net());
}


void WinMTRDialog::FocusHostCombo()
{
	m_comboHost.SetFocus();
}


void WinMTRDialog::RequestClose()
{
	OnOK();
}
