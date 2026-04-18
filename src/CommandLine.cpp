//*****************************************************************************
// FILE:            CommandLine.cpp
//
//*****************************************************************************

#include "Global.h"
#include "CommandLine.h"
#include "Dialog.h"
#include <string>

namespace {

int GetParamValue(LPTSTR cmd, const char* param, char sparam, char* value, size_t value_size)
{
	char* p;

	char p_long[1024];
	char p_short[1024];

	sprintf(p_long, "--%s ", param);
	sprintf(p_short, "-%c ", sparam);

	if ((p = strstr(cmd, p_long))) ;
	else
		p = strstr(cmd, p_short);

	if (p == NULL)
		return 0;

	if (strcmp(param, "numeric") == 0)
		return 1;

	while (*p && *p != ' ')
		p++;
	while (*p == ' ') p++;

	if (value_size == 0)
		return 1;
	size_t i = 0;
	while (*p && *p != ' ' && i < value_size - 1)
		value[i++] = *p++;
	value[i] = '\0';

	return 1;
}

int GetHostNameParamValue(LPTSTR cmd, std::string& host_name)
{
	// WinMTR -h -i 1 -n google.com
	int size = (int)strlen(cmd);
	std::string name = "";
	while (cmd[--size] == ' ');

	size++;
	while (size-- && cmd[size] != ' ' && (cmd[size] != '-' || cmd[size - 1] != ' ')) {
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
	if (cmd[size] == '-' && cmd[size - 1] == ' ') {
		// no target specified
		return 0;
	}

	std::string possible_argument = "";

	while (size-- && cmd[size] != ' ') {
		possible_argument = cmd[size] + possible_argument;
	}

	if (possible_argument.length() && (possible_argument[0] != '-' || possible_argument == "-n" || possible_argument == "--numeric")) {
		host_name = name;
		return 1;
	}

	return 0;
}

} // namespace


bool WinMTRCommandLine::Parse(LPTSTR cmd, WinMTRDialog* dlg)
{
	char value[1024];
	std::string host_name;

	if (GetParamValue(cmd, "help", 'h', value, sizeof(value))) {
		return true;
	}

	if (GetHostNameParamValue(cmd, host_name)) {
		dlg->SetHostName(host_name.c_str());
	}
	if (GetParamValue(cmd, "interval", 'i', value, sizeof(value))) {
		dlg->SetInterval((float)atof(value));
		dlg->hasIntervalFromCmdLine = true;
	}
	if (GetParamValue(cmd, "size", 's', value, sizeof(value))) {
		dlg->SetPingSize(atoi(value));
		dlg->hasPingsizeFromCmdLine = true;
	}
	if (GetParamValue(cmd, "maxLRU", 'm', value, sizeof(value))) {
		dlg->SetMaxLRU(atoi(value));
		dlg->hasMaxLRUFromCmdLine = true;
	}
	if (GetParamValue(cmd, "numeric", 'n', value, sizeof(value))) {
		dlg->SetUseDNS(FALSE);
		dlg->hasUseDNSFromCmdLine = true;
	}

	return false;
}
