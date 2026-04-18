//*****************************************************************************
// FILE:            TraceListView.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceListView.h"
#include "Net.h"
#include "MtrColumns.h"
#include <format>
#include <string>

namespace WinMTRTraceListView {

void InitColumns(CListCtrl& list)
{
	for (int i = 0; i < MTR_NR_COLS; ++i)
		list.InsertColumn(i, MTR_COLS[i], LVCFMT_LEFT, MTR_COL_LENGTH[i], -1);
}

void Refresh(CListCtrl& list, WinMTRNet& net)
{
	wchar_t buf[255];
	const int nh = net.GetMax();
	while (list.GetItemCount() > nh) list.DeleteItem(list.GetItemCount() - 1);

	const auto setCol = [&](int row, int col, const std::wstring& s) {
		list.SetItem(row, col, LVIF_TEXT, s.c_str(), 0, 0, 0, 0);
	};

	for (int i = 0; i < nh; ++i) {
		net.GetName(i, buf);
		if (buf[0] == L'\0') wcscpy_s(buf, L"No response from host");

		if (list.GetItemCount() <= i)
			list.InsertItem(i, buf);
		else
			list.SetItem(i, 0, LVIF_TEXT, buf, 0, 0, 0, 0);

		setCol(i, 1, std::format(L"{}", i + 1));
		setCol(i, 2, std::format(L"{}", net.GetPercent(i)));
		setCol(i, 3, std::format(L"{}", net.GetXmit(i)));
		setCol(i, 4, std::format(L"{}", net.GetReturned(i)));
		setCol(i, 5, std::format(L"{}", net.GetBest(i)));
		setCol(i, 6, std::format(L"{}", net.GetAvg(i)));
		setCol(i, 7, std::format(L"{}", net.GetWorst(i)));
		setCol(i, 8, std::format(L"{}", net.GetLast(i)));
	}
}

} // namespace WinMTRTraceListView
