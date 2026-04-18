//*****************************************************************************
// FILE:            HostResolver.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "HostResolver.h"

namespace {

bool ResolveByDns(LPCSTR hostname, int* outAddr, CString& errorMessage)
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	struct addrinfo* result = NULL;
	if (getaddrinfo(hostname, NULL, &hints, &result) != 0 || result == NULL) {
		errorMessage = "Unable to resolve hostname.";
		return false;
	}
	if (outAddr)
		*outAddr = (int)((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(result);
	return true;
}

} // namespace


//*****************************************************************************
// WinMTRHostResolver::LooksNumeric
//
//*****************************************************************************
bool WinMTRHostResolver::LooksNumeric(LPCSTR hostname)
{
	if (hostname == NULL) return false;
	for (const char* t = hostname; *t; ++t) {
		if (!isdigit((unsigned char)*t) && *t != '.') return false;
	}
	return true;
}


//*****************************************************************************
// WinMTRHostResolver::Validate
//
//*****************************************************************************
bool WinMTRHostResolver::Validate(LPCSTR hostname, CString& errorMessage)
{
	if (hostname == NULL) hostname = "localhost";
	if (LooksNumeric(hostname)) return true;
	return ResolveByDns(hostname, NULL, errorMessage);
}


//*****************************************************************************
// WinMTRHostResolver::Resolve
//
//*****************************************************************************
bool WinMTRHostResolver::Resolve(LPCSTR hostname, int& outAddr, CString& errorMessage)
{
	if (hostname == NULL) hostname = "localhost";
	if (LooksNumeric(hostname)) {
		struct in_addr addr;
		if (inet_pton(AF_INET, hostname, &addr) != 1) {
			errorMessage = "Invalid IP address.";
			return false;
		}
		outAddr = (int)addr.s_addr;
		return true;
	}
	return ResolveByDns(hostname, &outAddr, errorMessage);
}
