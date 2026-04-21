//*****************************************************************************
// FILE:            DialogLayout.h
//
// DESCRIPTION:
//   Resize/size-clamp logic for the main dialog's child controls.
//
//*****************************************************************************

#ifndef DIALOGLAYOUT_H_
#define DIALOGLAYOUT_H_

#include <afxwin.h>
#include <afxcmn.h>

namespace DialogLayout {

// Short-lived aggregate built at the call-site to bundle the controls whose
// bounds change together. Never copied or moved, so reference members are
// safe here; the cppcoreguidelines check flags the pattern generically.
// NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
struct ControlRefs {
	CStatic& staticS;
	CStatic& staticJ;
	CButton& buttonExit;
	CButton& buttonExpH;
	CButton& buttonExpT;
	CListCtrl& listMTR;
};
// NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

void AdjustInitialSize(CWnd& dialog);
void ApplyClientSize(CWnd& dialog, const ControlRefs& refs);
void ClampMinimum(LPRECT rect, const SIZE& minSize);

} // namespace DialogLayout

#endif // DIALOGLAYOUT_H_
