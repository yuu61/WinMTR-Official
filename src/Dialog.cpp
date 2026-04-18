//*****************************************************************************
// FILE:            Dialog.cpp
//*****************************************************************************

#include "Global.h"
#include "Dialog.h"
#include "Options.h"
#include "Properties.h"
#include "HopStatistics.h"
#include "Settings.h"
#include "Reporter.h"
#include "HostResolver.h"
#include "Version.h"
#include "DialogLayout.h"
#include "HostComboModel.h"
#include "StartTraceUseCase.h"
#include "TraceListView.h"
#include "TraceSessionController.h"
#include <memory>
#include <vector>
#include "afxlinkctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(Dialog, CDialog)
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
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, &Dialog::OnCbnSelchangeComboHost)
	ON_CBN_SELENDOK(IDC_COMBO_HOST, &Dialog::OnCbnSelendokComboHost)
	ON_CBN_CLOSEUP(IDC_COMBO_HOST, &Dialog::OnCbnCloseupComboHost)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &Dialog::OnBnClickedCancel)
	ON_MESSAGE(WM_WINMTR_TRACE_COMPLETED, &Dialog::OnTraceCompletedMsg)
	ON_MESSAGE(WM_WINMTR_TRACE_FAILED,    &Dialog::OnTraceFailedMsg)
END_MESSAGE_MAP()


Dialog::Dialog(CWnd* pParent)
	: CDialog(Dialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_autostart = 0;
	controller = std::make_unique<TraceSessionController>(this);
}

Dialog::~Dialog()
{
}


void Dialog::DoDataExchange(CDataExchange* pDX)
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


BOOL Dialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!controller->IsEngineValid()) {
		LPCWSTR err = controller->EngineError();
		AfxMessageBox(err ? err : L"ICMP subsystem initialization failed.");
	}

#ifndef _WIN64
	wchar_t caption[] = L"WinMTR v" WINMTR_VERSION L" 32 bit by Appnor MSP - www.winmtr.net";
#else
	wchar_t caption[] = L"WinMTR v" WINMTR_VERSION L" 64 bit by Appnor MSP - www.winmtr.net";
#endif

	SetTimer(1, WINMTR_DIALOG_TIMER, NULL);
	SetWindowText(caption);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	if (!statusBar.Setup(this, IDS_STRING_SB_NAME))
		AfxMessageBox(L"Error creating status bar");

	m_appnorLink.reset(new CMFCLinkCtrl);
	if (!statusBar.AddLinkPane(*m_appnorLink, L"www.appnor.com",
			L"http://www.appnor.com/?utm_source=winmtr&utm_medium=desktop&utm_campaign=software",
			1234, 100)) {
		AfxMessageBox(L"Failed to add status bar link pane", MB_ICONERROR);
		return FALSE;
	}

	TraceListView::InitColumns(m_listMTR);

	m_comboHost.SetFocus();

	DialogLayout::AdjustInitialSize(*this);

	{
		std::vector<CString> lruHosts;
		if (config.LoadAtInit(cmdline_overrides, lruHosts))
			HostComboModel::Populate(m_comboHost, lruHosts);
	}

	if (m_autostart) {
		m_comboHost.SetWindowText(msz_defaulthostname);
		OnRestart();
	}

	return FALSE;
}


void Dialog::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	DialogLayout::ClampMinimum(pRect);
}


void Dialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	DialogLayout::ControlRefs refs{
		m_staticS, m_staticJ, m_buttonExit, m_buttonExpH, m_buttonExpT, m_listMTR
	};
	DialogLayout::ApplyClientSize(*this, refs);
}


void Dialog::OnPaint()
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


HCURSOR Dialog::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void Dialog::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (!controller->IsTracing())
		return;

	POSITION pos = m_listMTR.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;

	int nItem = m_listMTR.GetNextSelectedItem(pos);
	Properties wmtrprop;
	wmtrprop.PopulateFrom(controller->Stats(), nItem);
	wmtrprop.DoModal();
}


void Dialog::SetHostName(const wchar_t* host)
{
	m_autostart = 1;
	wcsncpy_s(msz_defaulthostname, host, _TRUNCATE);
}


void Dialog::OnRestart()
{
	if (HostComboModel::IsSentinelSelected(m_comboHost)) {
		ClearHistory();
		return;
	}

	if (controller->CurrentState() != TraceSessionController::IDLE) {
		controller->RequestStop();
		return;
	}

	CString raw;
	m_comboHost.GetWindowText(raw);

	// Status hint before the blocking DNS probe.
	CString preview = raw;
	preview.TrimLeft();
	preview.TrimRight();
	if (!preview.IsEmpty() && !HostResolver::LooksNumeric(preview)) {
		statusBar.SetPaneText(0,
			std::format(L"Resolving host {}...", (LPCWSTR)preview).c_str());
	}

	m_listMTR.DeleteAllItems();

	auto result = StartTraceUseCase::Validate(raw);
	switch (result.validation) {
	case StartTraceUseCase::Validation::EmptyHost:
		AfxMessageBox(L"No host specified!");
		m_comboHost.SetFocus();
		return;
	case StartTraceUseCase::Validation::ResolutionFailed:
		statusBar.SetPaneText(0, CString((LPCWSTR)IDS_STRING_SB_NAME));
		AfxMessageBox(result.error);
		return;
	case StartTraceUseCase::Validation::Ok:
		break;
	}

	if (HostComboModel::AppendBeforeSentinel(m_comboHost, result.normalizedHost))
		config.lru.Append((LPCWSTR)result.normalizedHost);

	controller->RequestStart((LPCWSTR)result.normalizedHost, config.Snapshot());
}


void Dialog::OnOptions()
{
	Options optDlg;

	optDlg.SetPingSize(config.pingsize);
	optDlg.SetInterval(config.interval);
	optDlg.SetMaxLRU(config.lru.Max());
	optDlg.SetUseDNS(config.useDNS);

	if (IDOK == optDlg.DoModal()) {
		config.pingsize = optDlg.GetPingSize();
		config.interval = optDlg.GetInterval();
		config.lru.SetMax(optDlg.GetMaxLRU());
		config.useDNS   = optDlg.GetUseDNS();

		config.SaveOptions();
		if (config.lru.Max() < config.lru.Count())
			config.lru.Trim();
	}
}


void Dialog::OnCTTC()
{
	Reporter::CopyToClipboard(this, Reporter::BuildTextReport(controller->Stats()));
}


void Dialog::OnCHTC()
{
	Reporter::CopyToClipboard(this, Reporter::BuildHtmlReport(controller->Stats()));
}


void Dialog::OnEXPT()
{
	wchar_t BASED_CODE szFilter[] = L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L"TXT", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, szFilter, this);
	if (dlg.DoModal() == IDOK)
		Reporter::SaveToFile(dlg.GetPathName(), Reporter::BuildTextReport(controller->Stats()));
}


void Dialog::OnEXPH()
{
	wchar_t BASED_CODE szFilter[] = L"HTML Files (*.htm, *.html)|*.htm;*.html|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L"HTML", NULL, OFN_HIDEREADONLY | OFN_EXPLORER, szFilter, this);
	if (dlg.DoModal() == IDOK)
		Reporter::SaveToFile(dlg.GetPathName(), Reporter::BuildHtmlReport(controller->Stats()));
}


void Dialog::OnCancel()
{
}


void Dialog::OnCbnSelchangeComboHost()
{
}


void Dialog::ClearHistory()
{
	config.lru.Clear();
	HostComboModel::ClearAndResetSentinel(m_comboHost);
}


void Dialog::OnCbnSelendokComboHost()
{
}


void Dialog::OnCbnCloseupComboHost()
{
	if (HostComboModel::IsSentinelSelected(m_comboHost))
		ClearHistory();
}


void Dialog::OnTimer(UINT_PTR nIDEvent)
{
	controller->Tick();
	CDialog::OnTimer(nIDEvent);
}


void Dialog::OnClose()
{
	controller->RequestExit();
}


void Dialog::OnBnClickedCancel()
{
	controller->RequestExit();
}


void Dialog::SetStartEnabled(bool enabled)
{
	m_buttonStart.EnableWindow(enabled ? TRUE : FALSE);
}


void Dialog::SetStartText(LPCWSTR text)
{
	m_buttonStart.SetWindowText(text);
}


void Dialog::SetHostComboEnabled(bool enabled)
{
	m_comboHost.EnableWindow(enabled ? TRUE : FALSE);
}


void Dialog::SetOptionsEnabled(bool enabled)
{
	m_buttonOptions.EnableWindow(enabled ? TRUE : FALSE);
}


void Dialog::SetStatus(LPCWSTR text)
{
	statusBar.SetPaneText(0, text);
}


void Dialog::RefreshList()
{
	TraceListView::Refresh(m_listMTR, controller->Stats());
}


void Dialog::FocusHostCombo()
{
	m_comboHost.SetFocus();
}


void Dialog::RequestClose()
{
	OnOK();
}


void Dialog::ShowError(const CString& error)
{
	AfxMessageBox(error);
}


void Dialog::PostTraceCompleted()
{
	PostMessage(WM_WINMTR_TRACE_COMPLETED);
}


void Dialog::PostTraceFailed(const CString& error)
{
	auto* copy = new CString(error);
	if (!PostMessage(WM_WINMTR_TRACE_FAILED, 0, reinterpret_cast<LPARAM>(copy))) {
		delete copy;
	}
}


LRESULT Dialog::OnTraceCompletedMsg(WPARAM, LPARAM)
{
	controller->OnTraceCompleted();
	return 0;
}


LRESULT Dialog::OnTraceFailedMsg(WPARAM, LPARAM lParam)
{
	std::unique_ptr<CString> err(reinterpret_cast<CString*>(lParam));
	controller->OnTraceFailed(*err);
	return 0;
}
