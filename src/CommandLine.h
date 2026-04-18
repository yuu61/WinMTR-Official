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
	bool                        helpRequested = false;
	std::optional<std::wstring> hostName;
	CommandLineOverrides        overrides;
};

// Parses cmd into a ParseResult. On --help / -h, sets helpRequested and
// skips remaining parsing.
ParseResult Parse(LPWSTR cmd);

} // namespace CommandLine

#endif // COMMANDLINE_H_
