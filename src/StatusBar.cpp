#include "Global.h"
#include "StatusBar.h"
#include <afxlinkctrl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {
constexpr int kStatusBarMinHeight = 23;
constexpr int kEdgePaneMargin = 3;
} // namespace

StatusBar::~StatusBar()
{
	for (int i = 0; i < m_arrPaneControls.GetSize(); i++) {
		if (m_arrPaneControls[i]->hWnd && ::IsWindow(m_arrPaneControls[i]->hWnd)) {
			::ShowWindow(m_arrPaneControls[i]->hWnd, SW_HIDE);
			if (m_arrPaneControls[i]->bAutoDestroy) {
				::DestroyWindow(m_arrPaneControls[i]->hWnd);
			}
		}
		delete m_arrPaneControls[i];
	}
}

BEGIN_MESSAGE_MAP(StatusBar, CStatusBar)
	ON_WM_CREATE()
END_MESSAGE_MAP()


BOOL StatusBar::Setup(CWnd* parent, UINT titleStringId)
{
	if (!Create(parent)) {
		return FALSE;
	}
	GetStatusBarCtrl().SetMinHeight(kStatusBarMinHeight);
	UINT sbi[1]{};
	sbi[0] = titleStringId;
	SetIndicators(sbi, 1);
	SetPaneInfo(0, GetItemID(0), SBPS_STRETCH, NULL);
	return TRUE;
}

BOOL StatusBar::AddLinkPane(CMFCLinkCtrl& link, LPCWSTR text, LPCWSTR url,
                            UINT paneId, int width)
{
	if (!link.Create(text, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                 CRect(0, 0, 0, 0), this, paneId)) {
		return FALSE;
	}
	link.SetURL(url);
	if (!AddPane(paneId, 1)) {
		return FALSE;
	}
	SetPaneWidth(CommandToIndex(paneId), width);
	return AddPaneControl(&link, paneId, FALSE);
}

int StatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatusBar::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
	return 0;
}

LRESULT StatusBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	const LRESULT lResult = CStatusBar::WindowProc(message, wParam, lParam);
	if (message == WM_SIZE) {
		RepositionControls();
	}
	return lResult;
}

void StatusBar::RepositionControls()
{
	HDWP hDWP = ::BeginDeferWindowPos(static_cast<int>(m_arrPaneControls.GetSize()));

	CRect rcClient;
	GetClientRect(&rcClient);
	for (int i = 0; i < m_arrPaneControls.GetSize(); i++) {
		// clang-analyzer follows RemovePane's `delete m_arrPaneControls[i]; RemoveAt(i);`
		// into this loop and flags the access as use-after-free, but `RemoveAt` has
		// already dropped the freed entry from the array, so every element the loop
		// visits here is still live. Known analyzer limitation with MFC CArray.
		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
		const int iIndex = CommandToIndex(m_arrPaneControls[i]->nID);
		// `const HWND` would bind the const to the typedef'd pointer itself
		// (HWND__* const), not the pointee. Spell the pointer out so the
		// intent of a non-reassigned local is expressed without tripping
		// misc-misplaced-const / readability-qualified-auto.
		auto* const hWnd = m_arrPaneControls[i]->hWnd;

		CRect rcPane;
		GetItemRect(iIndex, &rcPane);

		// CStatusBar::GetItemRect() sometimes returns invalid size
		// of the last pane - we will re-compute it
		const int cx = ::GetSystemMetrics(SM_CXEDGE);
		const DWORD dwPaneStyle = GetPaneStyle(iIndex);
		if (iIndex == (m_nCount - 1)) {
			if ((dwPaneStyle & SBPS_STRETCH) == 0) {
				UINT nID = 0;
				UINT nStyle = 0;
				int cxWidth = 0;
				GetPaneInfo(iIndex, nID, nStyle, cxWidth);
				rcPane.right = rcPane.left + cxWidth + cx * kEdgePaneMargin;
			} else {
				GetClientRect(&rcClient);
				rcPane.right = rcClient.right;
				if ((GetStyle() & SBARS_SIZEGRIP) == SBARS_SIZEGRIP) {
					const int cxSmIcon = ::GetSystemMetrics(SM_CXSMICON);
					rcPane.right -= cxSmIcon + cx;
				}
			}
		}

		if ((GetPaneStyle(iIndex) & SBPS_NOBORDERS) == 0) {
			rcPane.DeflateRect(cx, cx);
		} else {
			rcPane.DeflateRect(cx, 1, cx, 1);
		}

		if (hWnd && ::IsWindow(hWnd)) {
			hDWP = ::DeferWindowPos(
			    hDWP,
			    hWnd,
			    nullptr,
			    rcPane.left,
			    rcPane.top,
			    rcPane.Width(),
			    rcPane.Height(),
			    SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);

			::RedrawWindow(
			    hWnd,
			    nullptr,
			    nullptr,
			    RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW);
		}
	}

	VERIFY(::EndDeferWindowPos(hDWP));
}

BOOL StatusBar::AddPane(UINT nID, int nIndex)
{
	if (nIndex < 0 || nIndex > m_nCount) {
		ASSERT(FALSE);
		return FALSE;
	}
	if (CommandToIndex(nID) != -1) {
		ASSERT(FALSE);
		return FALSE;
	}

	CArray<StatusBarPane*, StatusBarPane*> arrPanesTmp;
	int iIndex = 0;
	for (iIndex = 0; iIndex < m_nCount + 1; iIndex++) {
		StatusBarPane* pNewPane = new StatusBarPane;

		if (iIndex == nIndex) {
			pNewPane->nID = nID;
			pNewPane->nStyle = SBPS_NORMAL;
		} else {
			int idx = iIndex;
			if (iIndex > nIndex) {
				idx--;
			}
			StatusBarPane* pOldPane = GetPanePtr(idx);
			pNewPane->cxText = pOldPane->cxText;
			pNewPane->nFlags = pOldPane->nFlags;
			pNewPane->nID = pOldPane->nID;
			pNewPane->nStyle = pOldPane->nStyle;
			pNewPane->strText = pOldPane->strText;
		}
		arrPanesTmp.Add(pNewPane);
	}

	const int nPanesCount = static_cast<int>(arrPanesTmp.GetSize());
	UINT* lpIDArray = new UINT[nPanesCount];
	for (iIndex = 0; iIndex < nPanesCount; iIndex++) {
		lpIDArray[iIndex] = arrPanesTmp[iIndex]->nID;
	}

	SetIndicators(lpIDArray, nPanesCount);
	for (iIndex = 0; iIndex < nPanesCount; iIndex++) {
		StatusBarPane* pPane = arrPanesTmp[iIndex];
		if (iIndex != nIndex) {
			PaneInfoSet(iIndex, pPane);
		}
		delete pPane;
	}

	arrPanesTmp.RemoveAll();
	delete[] lpIDArray;

	RepositionControls();

	return TRUE;
}

BOOL StatusBar::RemovePane(UINT nID)
{
	if (CommandToIndex(nID) == -1 || m_nCount == 1) {
		ASSERT(FALSE);
		return FALSE;
	}

	CArray<StatusBarPane*, StatusBarPane*> arrPanesTmp;
	for (int nIndex = 0; nIndex < m_nCount; nIndex++) {
		StatusBarPane* pOldPane = GetPanePtr(nIndex);
		if (pOldPane->nID == nID) {
			continue;
		}
		StatusBarPane* pNewPane = new StatusBarPane;
		pNewPane->cxText = pOldPane->cxText;
		pNewPane->nFlags = pOldPane->nFlags;
		pNewPane->nID = pOldPane->nID;
		pNewPane->nStyle = pOldPane->nStyle;
		pNewPane->strText = pOldPane->strText;
		arrPanesTmp.Add(pNewPane);
	}

	UINT* lpIDArray = new UINT[arrPanesTmp.GetSize()];
	for (int nIndex = 0; nIndex < arrPanesTmp.GetSize(); nIndex++) {
		lpIDArray[nIndex] = arrPanesTmp[nIndex]->nID;
	}

	SetIndicators(lpIDArray, static_cast<int>(arrPanesTmp.GetSize()));
	for (int nIndex = 0; nIndex < arrPanesTmp.GetSize(); nIndex++) {
		StatusBarPane* pPane = arrPanesTmp[nIndex];
		PaneInfoSet(nIndex, pPane);
		delete pPane;
	}

	for (int i = 0; i < m_arrPaneControls.GetSize(); i++) {
		if (m_arrPaneControls[i]->nID == nID) {
			if (m_arrPaneControls[i]->hWnd && ::IsWindow(m_arrPaneControls[i]->hWnd)) {
				::ShowWindow(m_arrPaneControls[i]->hWnd, SW_HIDE);
				if (m_arrPaneControls[i]->bAutoDestroy) {
					::DestroyWindow(m_arrPaneControls[i]->hWnd);
				}
			}
			delete m_arrPaneControls[i];
			m_arrPaneControls.RemoveAt(i);
			break;
		}
	}

	arrPanesTmp.RemoveAll();
	delete[] lpIDArray;

	RepositionControls();

	return TRUE;
}

BOOL StatusBar::AddPaneControl(HWND hWnd, UINT nID, BOOL bAutoDestroy)
{
	if (CommandToIndex(nID) == -1) {
		return FALSE;
	}

	StatusBarPaneCtrl* pPaneCtrl = new StatusBarPaneCtrl;
	pPaneCtrl->nID = nID;
	pPaneCtrl->hWnd = hWnd;
	pPaneCtrl->bAutoDestroy = bAutoDestroy;

	m_arrPaneControls.Add(pPaneCtrl);

	RepositionControls();
	return TRUE;
}

BOOL StatusBar::PaneInfoGet(int nIndex, StatusBarPane* pPane)
{
	if (nIndex < m_nCount && nIndex >= 0) {
		GetPaneInfo(nIndex, pPane->nID, pPane->nStyle, pPane->cxText);
		CString strPaneText;
		GetPaneText(nIndex, strPaneText);
		pPane->strText = strPaneText.GetString();
		return TRUE;
	}
	return FALSE;
}

BOOL StatusBar::PaneInfoSet(int nIndex, StatusBarPane* pPane)
{
	if (nIndex < m_nCount && nIndex >= 0) {
		SetPaneInfo(nIndex, pPane->nID, pPane->nStyle, pPane->cxText);
		SetPaneText(nIndex, pPane->strText.GetString());
		return TRUE;
	}
	return FALSE;
}
