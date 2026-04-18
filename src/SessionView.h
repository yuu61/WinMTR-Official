//*****************************************************************************
// FILE:            SessionView.h
//
// DESCRIPTION:
//   View-side interface the TraceSessionController drives. Methods prefixed
//   with Post* are called from the session worker thread; implementations
//   must be thread-safe (typically post a window message to the UI thread).
//   The remaining methods are called on the UI thread.
//
//*****************************************************************************

#ifndef SESSIONVIEW_H_
#define SESSIONVIEW_H_

#include <afxwin.h>

class ISessionView {
public:
	virtual ~ISessionView() = default;

	// UI-thread only.
	virtual void SetStartEnabled(bool enabled)       = 0;
	virtual void SetStartText(LPCWSTR text)          = 0;
	virtual void SetHostComboEnabled(bool enabled)   = 0;
	virtual void SetOptionsEnabled(bool enabled)     = 0;
	virtual void SetStatus(LPCWSTR text)             = 0;
	virtual void RefreshList()                       = 0;
	virtual void FocusHostCombo()                    = 0;
	virtual void RequestClose()                      = 0;
	virtual void ShowError(const CString& error)     = 0;

	// Thread-safe; called from the session worker.
	virtual void PostTraceCompleted()                      = 0;
	virtual void PostTraceFailed(const CString& error)     = 0;
};

#endif // SESSIONVIEW_H_
