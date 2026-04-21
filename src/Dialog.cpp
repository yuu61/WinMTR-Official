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

namespace {
constexpr wchar_t kTxtFileFilter[] = L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
constexpr wchar_t kHtmlFileFilter[] = L"HTML Files (*.htm, *.html)|*.htm;*.html|All Files (*.*)|*.*||";
constexpr UINT_PTR kRefreshTimerId = 1;
constexpr UINT kAppnorLinkPaneId = 1234;
constexpr int kAppnorLinkPaneWidth = 100;
} // namespace

BEGIN_MESSAGE_MAP(Dialog, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_RESTART, &Dialog::OnRestart)
	ON_BN_CLICKED(ID_OPTIONS, &Dialog::OnOptions)
	ON_BN_CLICKED(ID_CTTC, &Dialog::OnCTTC)
	ON_BN_CLICKED(ID_CHTC, &Dialog::OnCHTC)
	ON_BN_CLICKED(ID_EXPT, &Dialog::OnEXPT)
	ON_BN_CLICKED(ID_EXPH, &Dialog::OnEXPH)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MTR, &Dialog::OnDblclkList)
	ON_CBN_CLOSEUP(IDC_COMBO_HOST, &Dialog::OnCbnCloseupComboHost)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &Dialog::OnBnClickedCancel)
	ON_MESSAGE(WM_WINMTR_TRACE_COMPLETED, &Dialog::OnTraceCompletedMsg)
	ON_MESSAGE(WM_WINMTR_TRACE_FAILED, &Dialog::OnTraceFailedMsg)
END_MESSAGE_MAP()


Dialog::Dialog(CWnd* pParent)
    : CDialog(Dialog::IDD, pParent),
      controller(std::make_unique<TraceSessionController>(this)),
      m_hIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME))
{
}

Dialog::~Dialog() = default;


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

	SetTimer(kRefreshTimerId, WINMTR_DIALOG_TIMER, nullptr);
	SetWindowText(caption);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	if (!statusBar.Setup(this, IDS_STRING_SB_NAME)) {
		AfxMessageBox(L"Error creating status bar");
		return FALSE;
	}

	m_appnorLink = std::make_unique<CMFCLinkCtrl>();
	if (!statusBar.AddLinkPane(*m_appnorLink, L"www.appnor.com",
	                           L"https://www.appnor.com/?utm_source=winmtr&utm_medium=desktop&utm_campaign=software",
	                           kAppnorLinkPaneId, kAppnorLinkPaneWidth)) {
		AfxMessageBox(L"Failed to add status bar link pane", MB_ICONERROR);
		return FALSE;
	}

	TraceListView::InitColumns(m_listMTR);

	m_comboHost.SetFocus();

	DialogLayout::AdjustInitialSize(*this);

	{
		// Record the dialog's initial window size as the minimum tracking size.
		// This keeps the template-defined layout as a hard lower bound regardless
		// of DPI or font metrics, preventing controls from overlapping on resize.
		CRect rcWindow;
		GetWindowRect(&rcWindow);
		m_minTrackSize = rcWindow.Size();
	}

	{
		std::vector<CString> lruHosts;
		if (config.LoadAtInit(cmdline_overrides, lruHosts)) {
			HostComboModel::Populate(m_comboHost, lruHosts);
		}
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
	DialogLayout::ClampMinimum(pRect, m_minTrackSize);
}


void Dialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (m_minTrackSize.cx > 0 && m_minTrackSize.cy > 0) {
		lpMMI->ptMinTrackSize.x = m_minTrackSize.cx;
		lpMMI->ptMinTrackSize.y = m_minTrackSize.cy;
	}
	CDialog::OnGetMinMaxInfo(lpMMI);
}


void Dialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	const DialogLayout::ControlRefs refs{
	    .staticS = m_staticS,
	    .staticJ = m_staticJ,
	    .buttonExit = m_buttonExit,
	    .buttonExpH = m_buttonExpH,
	    .buttonExpT = m_buttonExpT,
	    .listMTR = m_listMTR};
	DialogLayout::ApplyClientSize(*this, refs);
}


void Dialog::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		const int cxIcon = GetSystemMetrics(SM_CXICON);
		const int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		const int x = (rect.Width() - cxIcon + 1) / 2;
		const int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialog::OnPaint();
	}
}


HCURSOR Dialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void Dialog::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (!controller->IsTracing()) {
		return;
	}

	POSITION pos = m_listMTR.GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return;
	}

	const int nItem = m_listMTR.GetNextSelectedItem(pos);
	Properties wmtrprop;
	wmtrprop.PopulateFrom(controller->Stats(), nItem);
	wmtrprop.DoModal();
}


void Dialog::SetHostName(const wchar_t* host)
{
	m_autostart = true;
	wcsncpy_s(msz_defaulthostname, host, _TRUNCATE);
}


void Dialog::OnRestart()
{
	if (HostComboModel::IsSentinelSelected(m_comboHost)) {
		ClearHistory();
		return;
	}

	if (controller->CurrentState() != TraceSessionController::State::Idle) {
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
		                      std::format(L"Resolving host {}...", static_cast<LPCWSTR>(preview)).c_str());
	}

	m_listMTR.DeleteAllItems();

	auto result = StartTraceUseCase::Validate(raw);
	switch (result.validation) {
	case StartTraceUseCase::Validation::EmptyHost:
		AfxMessageBox(L"No host specified!");
		m_comboHost.SetFocus();
		return;
	case StartTraceUseCase::Validation::ResolutionFailed:
		statusBar.SetPaneText(0, CString(MAKEINTRESOURCE(IDS_STRING_SB_NAME)));
		AfxMessageBox(result.error);
		return;
	case StartTraceUseCase::Validation::Ok:
		break;
	}

	if (HostComboModel::AppendBeforeSentinel(m_comboHost, result.normalizedHost)) {
		config.lru.Append(static_cast<LPCWSTR>(result.normalizedHost));
	}

	controller->RequestStart(static_cast<LPCWSTR>(result.normalizedHost), config.Snapshot());
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
		config.useDNS = optDlg.GetUseDNS();

		config.SaveOptions();
		if (config.lru.Max() < config.lru.Count()) {
			config.lru.Trim();
		}
	}
}


void Dialog::OnCTTC()
{
	if (!Reporter::CopyToClipboard(this, Reporter::BuildTextReport(controller->Stats()))) {
		AfxMessageBox(L"Failed to copy report to clipboard.");
	}
}


void Dialog::OnCHTC()
{
	if (!Reporter::CopyToClipboard(this, Reporter::BuildHtmlReport(controller->Stats()))) {
		AfxMessageBox(L"Failed to copy report to clipboard.");
	}
}


void Dialog::OnEXPT()
{
	CFileDialog dlg(FALSE, L"TXT", nullptr, OFN_HIDEREADONLY | OFN_EXPLORER, kTxtFileFilter, this);
	if (dlg.DoModal() == IDOK) {
		if (!Reporter::SaveToFile(dlg.GetPathName(), Reporter::BuildTextReport(controller->Stats()))) {
			AfxMessageBox(L"Failed to save report to file.");
		}
	}
}


void Dialog::OnEXPH()
{
	CFileDialog dlg(FALSE, L"HTML", nullptr, OFN_HIDEREADONLY | OFN_EXPLORER, kHtmlFileFilter, this);
	if (dlg.DoModal() == IDOK) {
		if (!Reporter::SaveToFile(dlg.GetPathName(), Reporter::BuildHtmlReport(controller->Stats()))) {
			AfxMessageBox(L"Failed to save report to file.");
		}
	}
}


void Dialog::OnCancel()
{
	// Intentionally empty: Escape must not close the dialog.
	// Exit goes through the Exit button which triggers TraceSessionController::RequestExit().
}


void Dialog::ClearHistory()
{
	config.lru.Clear();
	HostComboModel::ClearAndResetSentinel(m_comboHost);
}


void Dialog::OnCbnCloseupComboHost()
{
	if (HostComboModel::IsSentinelSelected(m_comboHost)) {
		ClearHistory();
	}
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


// Ownership of `copy` transfers to the MFC message queue on PostMessage
// success; the UI thread reclaims and deletes it in OnTraceFailedMsg.
// clang-analyzer cannot model this hand-off and flags the success path as a
// leak, so this function is wrapped in a suppression block.
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
void Dialog::PostTraceFailed(const CString& error)
{
	try {
		auto* copy = new CString(error);
		// reinterpret_cast + int-to-ptr is the required MFC message-passing
		// protocol for carrying a heap pointer through WPARAM/LPARAM.
		// NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
		if (!PostMessage(WM_WINMTR_TRACE_FAILED, 0, reinterpret_cast<LPARAM>(copy))) {
			delete copy;
		}
	} catch (...) { // NOLINT(bugprone-empty-catch)
		            // The worker thread must never propagate an exception into MFC's
		            // message pump. On allocation/Post failure the error path is already
		            // logged upstream; dropping the notification here is the documented
		            // fallback.
	}
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)


LRESULT Dialog::OnTraceCompletedMsg(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	controller->OnTraceCompleted();
	return 0;
}


LRESULT Dialog::OnTraceFailedMsg(WPARAM /*wParam*/, LPARAM lParam)
{
	// Symmetric with PostTraceFailed: reclaim the heap CString encoded in lParam.
	// NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
	const std::unique_ptr<CString> err(reinterpret_cast<CString*>(lParam));
	controller->OnTraceFailed(*err);
	return 0;
}
