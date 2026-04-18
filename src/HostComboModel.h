//*****************************************************************************
// FILE:            HostComboModel.h
//
// DESCRIPTION:
//   Manages the host-history combo box, including the "Clear History"
//   sentinel row that always sits at the bottom of the list.
//
//*****************************************************************************

#ifndef HOSTCOMBOMODEL_H_
#define HOSTCOMBOMODEL_H_

#include <afxwin.h>
#include <vector>

namespace HostComboModel {

// Adds all hosts then appends the "Clear History" sentinel.
void Populate(CComboBox& combo, const std::vector<CString>& hosts);

// Removes every entry and restores the sentinel row.
void ClearAndResetSentinel(CComboBox& combo);

// Inserts host just above the sentinel if absent. Returns true if inserted.
bool AppendBeforeSentinel(CComboBox& combo, const CString& host);

// True if the combo's current selection is the sentinel row.
bool IsSentinelSelected(CComboBox& combo);

} // namespace HostComboModel

#endif // HOSTCOMBOMODEL_H_
