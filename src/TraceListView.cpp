//*****************************************************************************
// FILE:            TraceListView.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceListView.h"
#include "HopStatistics.h"
#include "MtrColumns.h"
#include <string>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

namespace TraceListView {

namespace {

constexpr int    kBaselineDpi     = 96;
constexpr size_t kHostBufferLen   = 255;

// Column layout indices (match MTR_COLS / MTR_COL_LENGTH ordering).
constexpr int kColHostname = 0;
constexpr int kColNr       = 1;
constexpr int kColLossPct  = 2;
constexpr int kColSent     = 3;
constexpr int kColRecv     = 4;
constexpr int kColBest     = 5;
constexpr int kColAvrg     = 6;
constexpr int kColWorst    = 7;
constexpr int kColLast     = 8;

} // namespace

void InitColumns(CListCtrl& list)
{
	const UINT dpi      = ::GetDpiForWindow(list.GetSafeHwnd());
	const int  dpiScale = dpi ? static_cast<int>(dpi) : kBaselineDpi;

	for (int i = 0; i < MTR_NR_COLS; ++i) {
		const int widthPx = ::MulDiv(MTR_COL_LENGTH[i], dpiScale, kBaselineDpi);
		list.InsertColumn(i, MTR_COLS[i], MTR_COL_FORMAT[i], widthPx, -1);
	}

	list.SetExtendedStyle(list.GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	::SetWindowTheme(list.GetSafeHwnd(), L"Explorer", nullptr);
}

void Refresh(CListCtrl& list, const HopStatistics& stats)
{
	wchar_t   buf[kHostBufferLen];
	const int nh = stats.GetMax();
	for (int n = list.GetItemCount(); n > nh; --n) {
		list.DeleteItem(n - 1);
	}

	const auto setCol = [&](int row, int col, const std::wstring& s) {
		list.SetItem(row, col, LVIF_TEXT, s.c_str(), 0, 0, 0, 0);
	};

	for (int i = 0; i < nh; ++i) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') {
			wcscpy_s(buf, L"No response from host");
		}

		if (list.GetItemCount() <= i) {
			list.InsertItem(i, buf);
		} else {
			list.SetItem(i, kColHostname, LVIF_TEXT, buf, 0, 0, 0, 0);
		}

		setCol(i, kColNr,      std::format(L"{}", i + 1));
		setCol(i, kColLossPct, std::format(L"{}", stats.GetPercent(i)));
		setCol(i, kColSent,    std::format(L"{}", stats.GetXmit(i)));
		setCol(i, kColRecv,    std::format(L"{}", stats.GetReturned(i)));
		setCol(i, kColBest,    std::format(L"{}", stats.GetBest(i)));
		setCol(i, kColAvrg,    std::format(L"{}", stats.GetAvg(i)));
		setCol(i, kColWorst,   std::format(L"{}", stats.GetWorst(i)));
		setCol(i, kColLast,    std::format(L"{}", stats.GetLast(i)));
	}
}

} // namespace TraceListView
