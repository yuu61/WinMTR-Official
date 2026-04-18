//*****************************************************************************
// FILE:            CommandLine.h
//
//
// DESCRIPTION:
//   Parses WinMTR command-line arguments and applies them to a dialog.
//
//*****************************************************************************

#ifndef WINMTRCOMMANDLINE_H_
#define WINMTRCOMMANDLINE_H_

#include <afxwin.h>

class WinMTRDialog;

namespace WinMTRCommandLine {

	// Parses cmd and applies settings onto dlg. Returns true if --help was
	// requested (the caller is expected to show the help dialog and exit).
	bool Parse(LPTSTR cmd, WinMTRDialog* dlg);

} // namespace WinMTRCommandLine

#endif // WINMTRCOMMANDLINE_H_
