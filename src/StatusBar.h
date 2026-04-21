#ifndef STATUSBAR_H_
#define STATUSBAR_H_

class CMFCLinkCtrl;

class StatusBar : public CStatusBar {
public:
	StatusBar() = default;
	~StatusBar() override;

	StatusBar(const StatusBar&) = delete;
	StatusBar& operator=(const StatusBar&) = delete;
	StatusBar(StatusBar&&) = delete;
	StatusBar& operator=(StatusBar&&) = delete;

	// Create the status bar, set min height, install a single stretch indicator.
	BOOL Setup(CWnd* parent, UINT titleStringId);

	// Create the link ctrl as a child of this bar, add a pane of the given
	// width, and register the ctrl as the pane body. Caller owns the CMFCLinkCtrl.
	BOOL AddLinkPane(CMFCLinkCtrl& link, LPCWSTR text, LPCWSTR url,
	                 UINT paneId, int width);

	[[nodiscard]] int GetPanesCount() const
	{
		return m_nCount;
	}

	void SetPaneWidth(int nIndex, int nWidth)
	{
		StatusBarPane pane;
		PaneInfoGet(nIndex, &pane);
		pane.cxText = nWidth;
		PaneInfoSet(nIndex, &pane);
	}

	BOOL AddPane(UINT nID, int nIndex);
	BOOL RemovePane(UINT nID);

	BOOL AddPaneControl(CWnd* pWnd, UINT nID, BOOL bAutoDestroy)
	{
		return AddPaneControl(pWnd->GetSafeHwnd(), nID, bAutoDestroy);
	}

	BOOL AddPaneControl(HWND hWnd, UINT nID, BOOL bAutoDestroy);

	void DisableControl(int nIndex, BOOL bDisable = TRUE)
	{
		const UINT uItemID = GetItemID(nIndex);
		for (int i = 0; i < m_arrPaneControls.GetSize(); i++) {
			if (uItemID == m_arrPaneControls[i]->nID) {
				if (m_arrPaneControls[i]->hWnd && ::IsWindow(m_arrPaneControls[i]->hWnd)) {
					::EnableWindow(m_arrPaneControls[i]->hWnd, bDisable);
				}
			}
		}
	}

	void SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth)
	{
		CStatusBar::SetPaneInfo(nIndex, nID, nStyle, cxWidth);
		const BOOL bDisabled = ((nStyle & SBPS_DISABLED) == 0);
		DisableControl(nIndex, bDisabled);
	}

	void SetPaneStyle(int nIndex, UINT nStyle)
	{
		CStatusBar::SetPaneStyle(nIndex, nStyle);
		const BOOL bDisabled = ((nStyle & SBPS_DISABLED) == 0);
		DisableControl(nIndex, bDisabled);
	}

protected:
	// Layout of StatusBarPane must match MFC's internal AFX_STATUSBAR_PANE
	// since GetPanePtr reinterprets m_pData (CStatusBar's pane array) as
	// StatusBarPane*. Do not reorder or insert members here.
	struct StatusBarPane {
		UINT nID = 0;    // IDC of indicator: 0 => normal text area
		int cxText = 0;  // width of string area in pixels (both sides add
		                 // a 3-pixel gap + 1-pixel border = pane is 6 wider)
		UINT nStyle = 0; // SBPS_* style flags
		UINT nFlags = 0; // SBPF_* state flags
		CString strText; // text in the pane
	};

	struct StatusBarPaneCtrl {
		HWND hWnd = nullptr;
		UINT nID = 0;
		BOOL bAutoDestroy = FALSE;
	};

	CArray<StatusBarPaneCtrl*, StatusBarPaneCtrl*> m_arrPaneControls;

	[[nodiscard]] StatusBarPane* GetPanePtr(int nIndex) const
	{
		ASSERT((nIndex >= 0 && nIndex < m_nCount) || m_nCount == 0);
		return static_cast<StatusBarPane*>(m_pData) + nIndex;
	}

	BOOL PaneInfoGet(int nIndex, StatusBarPane* pPane);
	BOOL PaneInfoSet(int nIndex, StatusBarPane* pPane);

	void RepositionControls();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
};

#endif // STATUSBAR_H_
