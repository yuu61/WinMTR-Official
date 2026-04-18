//*****************************************************************************
// FILE:            TraceListView.h
//
// DESCRIPTION:
//   Populates and refreshes the main hop list (CListCtrl) from HopStatistics.
//
//*****************************************************************************

#ifndef TRACELISTVIEW_H_
#define TRACELISTVIEW_H_

#include <afxwin.h>
#include <afxcmn.h>

class HopStatistics;

namespace TraceListView {

void InitColumns(CListCtrl& list);
void Refresh(CListCtrl& list, const HopStatistics& stats);

} // namespace TraceListView

#endif // TRACELISTVIEW_H_
