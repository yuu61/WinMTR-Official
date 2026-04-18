//*****************************************************************************
// FILE:            DialogLayout.cpp
//*****************************************************************************

#include "Global.h"
#include "DialogLayout.h"

namespace DialogLayout {

void AdjustInitialSize(CWnd& dialog)
{
	CRect rcClientStart;
	CRect rcClientNow;
	dialog.GetClientRect(rcClientStart);
	dialog.RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
	                      0, CWnd::reposQuery, rcClientNow);

	const CPoint ptOffset(rcClientNow.left - rcClientStart.left,
	                      rcClientNow.top  - rcClientStart.top);

	CRect rcChild;
	CWnd* pwndChild = dialog.GetWindow(GW_CHILD);
	while (pwndChild) {
		pwndChild->GetWindowRect(rcChild);
		dialog.ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	CRect rcWindow;
	dialog.GetWindowRect(rcWindow);
	rcWindow.right  += rcClientStart.Width()  - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	dialog.MoveWindow(rcWindow, FALSE);

	dialog.RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

void ApplyClientSize(CWnd& dialog, const ControlRefs& refs)
{
	CRect r;
	dialog.GetClientRect(&r);
	CRect lb;

	if (::IsWindow(refs.staticS.m_hWnd)) {
		refs.staticS.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.staticS.SetWindowPos(NULL, lb.TopLeft().x, lb.TopLeft().y,
			r.Width() - lb.TopLeft().x - 10, lb.Height(),
			SWP_NOMOVE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.staticJ.m_hWnd)) {
		refs.staticJ.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.staticJ.SetWindowPos(NULL, lb.TopLeft().x, lb.TopLeft().y,
			r.Width() - 21, lb.Height(),
			SWP_NOMOVE | SWP_NOZORDER);
	}

	// Right-edge anchor margins (pixels at the dialog's current DPI).
	// Exit and ExpH share the 7 DLU right margin; ExpT leaves room for ExpH + gap.
	const UINT dpi        = ::GetDpiForWindow(dialog.GetSafeHwnd());
	const int  dpiScale   = dpi ? static_cast<int>(dpi) : 96;
	const int  marginEdge = ::MulDiv(11,  dpiScale, 96);  // ~7 DLU
	const int  marginExpT = ::MulDiv(120, dpiScale, 96);  // ~80 DLU (ExpH width + 8 DLU gap + 7 DLU margin)

	if (::IsWindow(refs.buttonExit.m_hWnd)) {
		refs.buttonExit.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExit.SetWindowPos(NULL, r.Width() - lb.Width() - marginEdge, lb.TopLeft().y,
			lb.Width(), lb.Height(),
			SWP_NOSIZE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.buttonExpH.m_hWnd)) {
		refs.buttonExpH.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExpH.SetWindowPos(NULL, r.Width() - lb.Width() - marginEdge, lb.TopLeft().y,
			lb.Width(), lb.Height(),
			SWP_NOSIZE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.buttonExpT.m_hWnd)) {
		refs.buttonExpT.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExpT.SetWindowPos(NULL, r.Width() - lb.Width() - marginExpT, lb.TopLeft().y,
			lb.Width(), lb.Height(),
			SWP_NOSIZE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.listMTR.m_hWnd)) {
		refs.listMTR.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.listMTR.SetWindowPos(NULL, lb.TopLeft().x, lb.TopLeft().y,
			r.Width() - 21, r.Height() - lb.top - 25,
			SWP_NOMOVE | SWP_NOZORDER);
	}

	dialog.RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
		0, CWnd::reposQuery, r);
	dialog.RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

void ClampMinimum(LPRECT rect, const SIZE& minSize)
{
	if (rect->right - rect->left < minSize.cx) rect->right  = rect->left + minSize.cx;
	if (rect->bottom - rect->top < minSize.cy) rect->bottom = rect->top  + minSize.cy;
}

} // namespace DialogLayout
