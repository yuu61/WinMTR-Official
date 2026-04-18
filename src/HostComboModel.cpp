//*****************************************************************************
// FILE:            HostComboModel.cpp
//*****************************************************************************

#include "Global.h"
#include "HostComboModel.h"

namespace {

inline CString SentinelText()
{
	return CString((LPCWSTR)IDS_STRING_CLEAR_HISTORY);
}

} // namespace

namespace HostComboModel {

void Populate(CComboBox& combo, const std::vector<CString>& hosts)
{
	for (const auto& h : hosts)
		combo.AddString(h);
	combo.AddString(SentinelText());
}

void ClearAndResetSentinel(CComboBox& combo)
{
	combo.Clear();
	combo.ResetContent();
	combo.AddString(SentinelText());
}

bool AppendBeforeSentinel(CComboBox& combo, const CString& host)
{
	if (combo.FindString(-1, host) != CB_ERR)
		return false;
	combo.InsertString(combo.GetCount() - 1, host);
	return true;
}

bool IsSentinelSelected(CComboBox& combo)
{
	return combo.GetCurSel() == combo.GetCount() - 1;
}

} // namespace HostComboModel
