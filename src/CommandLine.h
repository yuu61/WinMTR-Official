//*****************************************************************************
// FILE:            CommandLine.h
//
//
// DESCRIPTION:
//   Parses WinMTR command-line arguments into a ParseResult the caller can
//   apply to the dialog / config.
//
//*****************************************************************************

#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_

#include "CommandLineOverrides.h"
#include <afxwin.h>
#include <optional>
#include <string>

namespace CommandLine {

struct ParseResult {
	bool helpRequested = false;
	std::optional<std::wstring> hostName;
	CommandLineOverrides overrides;
};

// Parses the full command line string (e.g. as returned by GetCommandLineW())
// into a ParseResult. Splits via CommandLineToArgvW so quoting and escaping
// follow standard Windows shell semantics. On --help / -h, sets helpRequested
// and skips the remainder.
[[nodiscard]] ParseResult Parse(LPCWSTR cmdLine);

} // namespace CommandLine

#endif // COMMANDLINE_H_
