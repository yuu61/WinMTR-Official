//*****************************************************************************
// FILE:            CommandLine.cpp
//*****************************************************************************

#include "Global.h"
#include "CommandLine.h"

#include <shellapi.h>
#include <string_view>
#include <cwchar>

#pragma comment(lib, "shell32.lib")

namespace {

using std::wstring_view;

bool Matches(wstring_view a, wstring_view shortFlag, wstring_view longFlag)
{
	return a == shortFlag || a == longFlag;
}

bool TryParseInt(wstring_view s, int& out)
{
	if (s.empty()) return false;
	wchar_t* end = nullptr;
	errno = 0;
	const long v = std::wcstol(s.data(), &end, 10);
	if (end == s.data() || errno == ERANGE) return false;
	out = static_cast<int>(v);
	return true;
}

bool TryParseDouble(wstring_view s, double& out)
{
	if (s.empty()) return false;
	wchar_t* end = nullptr;
	errno = 0;
	const double v = std::wcstod(s.data(), &end);
	if (end == s.data() || errno == ERANGE) return false;
	out = v;
	return true;
}

} // namespace


CommandLine::ParseResult CommandLine::Parse(LPCWSTR cmdLine)
{
	ParseResult result;
	if (cmdLine == nullptr) return result;

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(cmdLine, &argc);
	if (argv == nullptr) return result;

	// argv[0] is the program name per Windows shell conventions; skip it.
	for (int i = 1; i < argc; ++i) {
		const wstring_view a{argv[i]};

		if (Matches(a, L"-h", L"--help")) {
			result.helpRequested = true;
			break;
		}

		if (Matches(a, L"-n", L"--numeric")) {
			result.overrides.useDNS = FALSE;
			continue;
		}

		if (Matches(a, L"-i", L"--interval") && i + 1 < argc) {
			double v = 0.0;
			if (TryParseDouble(argv[++i], v) && v >= 0.1 && v <= 60.0)
				result.overrides.interval = v;
			continue;
		}

		if (Matches(a, L"-s", L"--size") && i + 1 < argc) {
			int v = 0;
			if (TryParseInt(argv[++i], v) && v >= 0 && v <= 8184)
				result.overrides.pingsize = v;
			continue;
		}

		if (Matches(a, L"-m", L"--maxLRU") && i + 1 < argc) {
			int v = 0;
			if (TryParseInt(argv[++i], v) && v >= 1 && v <= 10000)
				result.overrides.maxLRU = v;
			continue;
		}

		// Any non-flag positional is treated as the host name. Later
		// positionals overwrite earlier ones, matching the legacy "last
		// word wins" behavior.
		if (!a.empty() && a.front() != L'-')
			result.hostName = std::wstring(a);
	}

	LocalFree(argv);
	return result;
}
