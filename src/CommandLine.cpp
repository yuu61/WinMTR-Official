//*****************************************************************************
// FILE:            CommandLine.cpp
//*****************************************************************************

#include "Global.h"
#include "CommandLine.h"
#include <string>

namespace {

int GetParamValue(LPWSTR cmd, const wchar_t* param, wchar_t sparam, wchar_t* value, size_t value_size)
{
	wchar_t* p;

	const std::wstring p_long  = std::format(L"--{} ", param);
	const std::wstring p_short = std::format(L"-{} ", sparam);

	if ((p = wcsstr(cmd, p_long.c_str()))) ;
	else
		p = wcsstr(cmd, p_short.c_str());

	if (p == NULL)
		return 0;

	if (wcscmp(param, L"numeric") == 0)
		return 1;

	while (*p && *p != L' ')
		p++;
	while (*p == L' ') p++;

	if (value_size == 0)
		return 1;
	size_t i = 0;
	while (*p && *p != L' ' && i < value_size - 1)
		value[i++] = *p++;
	value[i] = L'\0';

	return 1;
}

int GetHostNameParamValue(LPWSTR cmd, std::wstring& host_name)
{
	// WinMTR -h -i 1 -n google.com
	int size = (int)wcslen(cmd);
	std::wstring name = L"";
	while (cmd[--size] == L' ');

	size++;
	while (size-- && cmd[size] != L' ' && (cmd[size] != L'-' || cmd[size - 1] != L' ')) {
		name = cmd[size] + name;
	}

	if (size == -1) {
		if (name.length() == 0) {
			return 0;
		} else {
			host_name = name;
			return 1;
		}
	}
	if (cmd[size] == L'-' && cmd[size - 1] == L' ') {
		// no target specified
		return 0;
	}

	std::wstring possible_argument = L"";

	while (size-- && cmd[size] != L' ') {
		possible_argument = cmd[size] + possible_argument;
	}

	if (possible_argument.length() && (possible_argument[0] != L'-' || possible_argument == L"-n" || possible_argument == L"--numeric")) {
		host_name = name;
		return 1;
	}

	return 0;
}

} // namespace


WinMTRCommandLine::ParseResult WinMTRCommandLine::Parse(LPWSTR cmd)
{
	ParseResult result;
	wchar_t value[1024];

	if (GetParamValue(cmd, L"help", L'h', value, _countof(value))) {
		result.helpRequested = true;
		return result;
	}

	std::wstring host_name;
	if (GetHostNameParamValue(cmd, host_name)) {
		result.hostName = host_name;
	}
	if (GetParamValue(cmd, L"interval", L'i', value, _countof(value))) {
		result.overrides.interval = (double)_wtof(value);
	}
	if (GetParamValue(cmd, L"size", L's', value, _countof(value))) {
		result.overrides.pingsize = _wtoi(value);
	}
	if (GetParamValue(cmd, L"maxLRU", L'm', value, _countof(value))) {
		result.overrides.maxLRU = _wtoi(value);
	}
	if (GetParamValue(cmd, L"numeric", L'n', value, _countof(value))) {
		result.overrides.useDNS = FALSE;
	}

	return result;
}
