//*****************************************************************************
// FILE:            CommandLineOverrides.h
//
// DESCRIPTION:
//   Optional command-line overrides for trace configuration. A populated
//   std::optional means "the user passed this flag"; empty means "fall back
//   to persisted or default value".
//
//*****************************************************************************

#ifndef COMMANDLINEOVERRIDES_H_
#define COMMANDLINEOVERRIDES_H_

#include <windef.h>
#include <optional>

struct CommandLineOverrides {
	std::optional<double> interval;
	std::optional<int>    pingsize;
	std::optional<int>    maxLRU;
	std::optional<BOOL>   useDNS;
};

#endif // COMMANDLINEOVERRIDES_H_
