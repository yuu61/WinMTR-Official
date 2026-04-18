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

struct ControlRefs {
	CStatic&   staticS;
	CStatic&   staticJ;
	CButton&   buttonExit;
	CButton&   buttonExpH;
	CButton&   buttonExpT;
	CListCtrl& listMTR;
};

void AdjustInitialSize(CWnd& dialog);
void ApplyClientSize(CWnd& dialog, const ControlRefs& refs);
void ClampMinimum(LPRECT rect);

} // namespace DialogLayout

#endif // DIALOGLAYOUT_H_
