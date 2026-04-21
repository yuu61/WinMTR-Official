//*****************************************************************************
// FILE:            CommandLine.cpp
//*****************************************************************************

#include "Global.h"
#include "CommandLine.h"

#include <shellapi.h>
#include <string_view>
#include <cwchar>
#include <cerrno>

#pragma comment(lib, "shell32.lib")

namespace {

using std::wstring_view;

constexpr double kMinInterval = 0.1;
constexpr double kMaxInterval = 60.0;
constexpr int kMinPingSize = 0;
constexpr int kMaxPingSize = 8184;
constexpr int kMinMaxLRU = 1;
constexpr int kMaxMaxLRU = 10000;
constexpr int kWcstolBase = 10;

bool Matches(wstring_view a, wstring_view shortFlag, wstring_view longFlag)
{
	return a == shortFlag || a == longFlag;
}

// argv[i] is always null-terminated per CommandLineToArgvW, so TryParse*
// takes a raw pointer to avoid passing a non-terminated string_view::data()
// to the C string-to-number routines.
bool TryParseInt(const wchar_t* s, int& out)
{
	if (s == nullptr || *s == L'\0') {
		return false;
	}
	wchar_t* end = nullptr;
	errno = 0;
	const long v = std::wcstol(s, &end, kWcstolBase);
	if (end == s || errno == ERANGE) {
		return false;
	}
	out = static_cast<int>(v);
	return true;
}

bool TryParseDouble(const wchar_t* s, double& out)
{
	if (s == nullptr || *s == L'\0') {
		return false;
	}
	wchar_t* end = nullptr;
	errno = 0;
	const double v = std::wcstod(s, &end);
	if (end == s || errno == ERANGE) {
		return false;
	}
	out = v;
	return true;
}

// Per-flag handlers. Each returns true when it has consumed the current
// argument (and any value that follows); the caller should `continue` the
// outer loop. Splitting these out keeps CommandLine::Parse's cognitive
// complexity under the strict clang-tidy threshold.
bool TryHandleInterval(wstring_view a, int& i, int argc, LPWSTR* argv, CommandLineOverrides& ov)
{
	if (!Matches(a, L"-i", L"--interval") || i + 1 >= argc) {
		return false;
	}
	double v = 0.0;
	if (TryParseDouble(argv[++i], v) && v >= kMinInterval && v <= kMaxInterval) {
		ov.interval = v;
	}
	return true;
}

bool TryHandlePingSize(wstring_view a, int& i, int argc, LPWSTR* argv, CommandLineOverrides& ov)
{
	if (!Matches(a, L"-s", L"--size") || i + 1 >= argc) {
		return false;
	}
	int v = 0;
	if (TryParseInt(argv[++i], v) && v >= kMinPingSize && v <= kMaxPingSize) {
		ov.pingsize = v;
	}
	return true;
}

bool TryHandleMaxLRU(wstring_view a, int& i, int argc, LPWSTR* argv, CommandLineOverrides& ov)
{
	if (!Matches(a, L"-m", L"--maxLRU") || i + 1 >= argc) {
		return false;
	}
	int v = 0;
	if (TryParseInt(argv[++i], v) && v >= kMinMaxLRU && v <= kMaxMaxLRU) {
		ov.maxLRU = v;
	}
	return true;
}

} // namespace


CommandLine::ParseResult CommandLine::Parse(LPCWSTR cmdLine)
{
	ParseResult result;
	if (cmdLine == nullptr) {
		return result;
	}

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(cmdLine, &argc);
	if (argv == nullptr) {
		return result;
	}

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
		if (TryHandleInterval(a, i, argc, argv, result.overrides)) {
			continue;
		}
		if (TryHandlePingSize(a, i, argc, argv, result.overrides)) {
			continue;
		}
		if (TryHandleMaxLRU(a, i, argc, argv, result.overrides)) {
			continue;
		}

		// Any non-flag positional is treated as the host name. Later
		// positionals overwrite earlier ones, matching the legacy "last
		// word wins" behavior.
		if (!a.empty() && a.front() != L'-') {
			result.hostName = std::wstring(a);
		}
	}

	LocalFree(static_cast<HLOCAL>(argv));
	return result;
}
