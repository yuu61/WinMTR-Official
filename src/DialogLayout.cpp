//*****************************************************************************
// FILE:            DialogLayout.cpp
//*****************************************************************************

#include "Global.h"
#include "DialogLayout.h"

namespace DialogLayout {

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

	if (::IsWindow(refs.buttonExit.m_hWnd)) {
		refs.buttonExit.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExit.SetWindowPos(NULL, r.Width() - lb.Width() - 21, lb.TopLeft().y,
			lb.Width(), lb.Height(),
			SWP_NOSIZE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.buttonExpH.m_hWnd)) {
		refs.buttonExpH.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExpH.SetWindowPos(NULL, r.Width() - lb.Width() - 21, lb.TopLeft().y,
			lb.Width(), lb.Height(),
			SWP_NOSIZE | SWP_NOZORDER);
	}

	if (::IsWindow(refs.buttonExpT.m_hWnd)) {
		refs.buttonExpT.GetWindowRect(&lb);
		dialog.ScreenToClient(&lb);
		refs.buttonExpT.SetWindowPos(NULL, r.Width() - lb.Width() - 103, lb.TopLeft().y,
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

void ClampMinimum(LPRECT rect)
{
	const int iWidth  = rect->right - rect->left;
	const int iHeight = rect->bottom - rect->top;
	if (iWidth  < 600) rect->right  = rect->left + 600;
	if (iHeight < 250) rect->bottom = rect->top  + 250;
}

} // namespace DialogLayout
