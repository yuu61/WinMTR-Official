//*****************************************************************************
// FILE:            CommandLine.cpp
//*****************************************************************************

#include "Global.h"
#include "CommandLine.h"
#include "Dialog.h"
#include <string>

namespace {

int GetParamValue(LPWSTR cmd, const wchar_t* param, wchar_t sparam, wchar_t* value, size_t value_size)
{
	wchar_t* p;

	wchar_t p_long[1024];
	wchar_t p_short[1024];

	swprintf(p_long, 1024, L"--%s ", param);
	swprintf(p_short, 1024, L"-%c ", sparam);

	if ((p = wcsstr(cmd, p_long))) ;
	else
		p = wcsstr(cmd, p_short);

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


bool WinMTRCommandLine::Parse(LPWSTR cmd, WinMTRDialog* dlg)
{
	wchar_t value[1024];
	std::wstring host_name;

	if (GetParamValue(cmd, L"help", L'h', value, _countof(value))) {
		return true;
	}

	if (GetHostNameParamValue(cmd, host_name)) {
		dlg->SetHostName(host_name.c_str());
	}
	if (GetParamValue(cmd, L"interval", L'i', value, _countof(value))) {
		dlg->SetInterval((float)_wtof(value));
		dlg->hasIntervalFromCmdLine = true;
	}
	if (GetParamValue(cmd, L"size", L's', value, _countof(value))) {
		dlg->SetPingSize(_wtoi(value));
		dlg->hasPingsizeFromCmdLine = true;
	}
	if (GetParamValue(cmd, L"maxLRU", L'm', value, _countof(value))) {
		dlg->SetMaxLRU(_wtoi(value));
		dlg->hasMaxLRUFromCmdLine = true;
	}
	if (GetParamValue(cmd, L"numeric", L'n', value, _countof(value))) {
		dlg->SetUseDNS(FALSE);
		dlg->hasUseDNSFromCmdLine = true;
	}

	return false;
}
