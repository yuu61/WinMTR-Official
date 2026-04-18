//*****************************************************************************
// FILE:            TraceListView.h
//
// DESCRIPTION:
//   Populates and refreshes the main hop list (CListCtrl) from WinMTRNet.
//
//*****************************************************************************

#ifndef TRACELISTVIEW_H_
#define TRACELISTVIEW_H_

#include <afxwin.h>
#include <afxcmn.h>

class WinMTRNet;

namespace WinMTRTraceListView {

void InitColumns(CListCtrl& list);
void Refresh(CListCtrl& list, WinMTRNet& net);

} // namespace WinMTRTraceListView

#endif // TRACELISTVIEW_H_
