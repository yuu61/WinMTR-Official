//*****************************************************************************
// FILE:            Dialog.h
//
// DESCRIPTION:
//   Main WinMTR dialog. Acts as a facade that owns the config state plus a
//   TraceSessionController, and fulfills ISessionView for the controller.
//
//*****************************************************************************

#ifndef WINMTRDIALOG_H_
#define WINMTRDIALOG_H_

#include "StatusBar.h"
#include "SessionView.h"
#include "TraceConfigState.h"
#include <memory>

constexpr UINT WINMTR_DIALOG_TIMER      = 100;
constexpr UINT WM_WINMTR_TRACE_COMPLETED = WM_USER + 100;
constexpr UINT WM_WINMTR_TRACE_FAILED    = WM_USER + 101;

class CMFCLinkCtrl;
class TraceSessionController;

//*****************************************************************************
// CLASS:  WinMTRDialog
//*****************************************************************************

class WinMTRDialog : public CDialog, public ISessionView
{
public:
	WinMTRDialog(CWnd* pParent = NULL);
	~WinMTRDialog() override;

	enum { IDD = IDD_WINMTR_DIALOG };

	WinMTRStatusBar statusBar;

	CButton   m_buttonOptions;
	CButton   m_buttonExit;
	CButton   m_buttonStart;
	CComboBox m_comboHost;
	CListCtrl m_listMTR;

	CStatic m_staticS;
	CStatic m_staticJ;

	CButton m_buttonExpT;
	CButton m_buttonExpH;

	std::unique_ptr<CMFCLinkCtrl> m_appnorLink;

	void SetHostName(const wchar_t* host);

	[[nodiscard]] TraceConfigState&       Config()       { return config; }
	[[nodiscard]] const TraceConfigState& Config() const { return config; }

	// ISessionView
	void SetStartEnabled(bool enabled)             override;
	void SetStartText(LPCWSTR text)                override;
	void SetHostComboEnabled(bool enabled)         override;
	void SetOptionsEnabled(bool enabled)           override;
	void SetStatus(LPCWSTR text)                   override;
	void RefreshList()                             override;
	void FocusHostCombo()                          override;
	void RequestClose()                            override;
	void ShowError(const CString& error)           override;
	void PostTraceCompleted()                      override;
	void PostTraceFailed(const CString& error)     override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	TraceConfigState                        config;
	std::unique_ptr<TraceSessionController> controller;

	int     m_autostart;
	wchar_t msz_defaulthostname[1000]{};
	HICON   m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT, int, int);
	afx_msg void OnSizing(UINT, LPRECT);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRestart();
	afx_msg void OnOptions();
	virtual void OnCancel();

	afx_msg void OnCTTC();
	afx_msg void OnCHTC();
	afx_msg void OnEXPT();
	afx_msg void OnEXPH();

	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboHost();
	afx_msg void OnCbnSelendokComboHost();
private:
	void ClearHistory();
public:
	afx_msg void OnCbnCloseupComboHost();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg LRESULT OnTraceCompletedMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTraceFailedMsg(WPARAM wParam, LPARAM lParam);
};

#endif // WINMTRDIALOG_H_
