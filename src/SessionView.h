//*****************************************************************************
// FILE:            SessionView.h
//
// DESCRIPTION:
//   View-side interface the TraceSessionController pushes UI effects through.
//
//*****************************************************************************

#ifndef SESSIONVIEW_H_
#define SESSIONVIEW_H_

#include <afxwin.h>

class ISessionView {
public:
	virtual ~ISessionView() = default;

	virtual void SetStartEnabled(bool enabled)       = 0;
	virtual void SetStartText(LPCWSTR text)          = 0;
	virtual void SetHostComboEnabled(bool enabled)   = 0;
	virtual void SetOptionsEnabled(bool enabled)     = 0;
	virtual void SetStatus(LPCWSTR text)             = 0;
	virtual void RefreshList()                       = 0;
	virtual void FocusHostCombo()                    = 0;
	virtual void RequestClose()                      = 0;
};

#endif // SESSIONVIEW_H_
