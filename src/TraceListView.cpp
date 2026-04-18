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

void InitColumns(CListCtrl& list)
{
	const UINT dpi      = ::GetDpiForWindow(list.GetSafeHwnd());
	const int  dpiScale = dpi ? static_cast<int>(dpi) : 96;

	for (int i = 0; i < MTR_NR_COLS; ++i) {
		const int widthPx = ::MulDiv(MTR_COL_LENGTH[i], dpiScale, 96);
		list.InsertColumn(i, MTR_COLS[i], MTR_COL_FORMAT[i], widthPx, -1);
	}

	list.SetExtendedStyle(list.GetExtendedStyle()
		| LVS_EX_DOUBLEBUFFER
		| LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP);

	::SetWindowTheme(list.GetSafeHwnd(), L"Explorer", nullptr);
}

void Refresh(CListCtrl& list, const HopStatistics& stats)
{
	wchar_t buf[255];
	const int nh = stats.GetMax();
	while (list.GetItemCount() > nh) list.DeleteItem(list.GetItemCount() - 1);

	const auto setCol = [&](int row, int col, const std::wstring& s) {
		list.SetItem(row, col, LVIF_TEXT, s.c_str(), 0, 0, 0, 0);
	};

	for (int i = 0; i < nh; ++i) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') wcscpy_s(buf, L"No response from host");

		if (list.GetItemCount() <= i)
			list.InsertItem(i, buf);
		else
			list.SetItem(i, 0, LVIF_TEXT, buf, 0, 0, 0, 0);

		setCol(i, 1, std::format(L"{}", i + 1));
		setCol(i, 2, std::format(L"{}", stats.GetPercent(i)));
		setCol(i, 3, std::format(L"{}", stats.GetXmit(i)));
		setCol(i, 4, std::format(L"{}", stats.GetReturned(i)));
		setCol(i, 5, std::format(L"{}", stats.GetBest(i)));
		setCol(i, 6, std::format(L"{}", stats.GetAvg(i)));
		setCol(i, 7, std::format(L"{}", stats.GetWorst(i)));
		setCol(i, 8, std::format(L"{}", stats.GetLast(i)));
	}
}

} // namespace TraceListView
