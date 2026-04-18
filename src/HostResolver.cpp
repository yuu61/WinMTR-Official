//*****************************************************************************
// FILE:            HostResolver.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "HostResolver.h"
#include <algorithm>
#include <string_view>

namespace {

bool ResolveByDns(LPCWSTR hostname, int* outAddr, CString& errorMessage)
{
	ADDRINFOW hints = {};
	hints.ai_family = AF_INET;
	PADDRINFOW result = NULL;
	if (GetAddrInfoW(hostname, NULL, &hints, &result) != 0 || result == NULL) {
		errorMessage = L"Unable to resolve hostname.";
		return false;
	}
	if (outAddr)
		*outAddr = (int)((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
	FreeAddrInfoW(result);
	return true;
}

} // namespace


//*****************************************************************************
// HostResolver::LooksNumeric
//
//*****************************************************************************
bool HostResolver::LooksNumeric(LPCWSTR hostname)
{
	if (hostname == nullptr) return false;
	return std::ranges::all_of(std::wstring_view{hostname},
		[](wchar_t c) { return iswdigit(c) || c == L'.'; });
}


//*****************************************************************************
// HostResolver::Validate
//
//*****************************************************************************
bool HostResolver::Validate(LPCWSTR hostname, CString& errorMessage)
{
	if (hostname == NULL) hostname = L"localhost";
	if (LooksNumeric(hostname)) return true;
	return ResolveByDns(hostname, NULL, errorMessage);
}


//*****************************************************************************
// HostResolver::Resolve
//
//*****************************************************************************
bool HostResolver::Resolve(LPCWSTR hostname, int& outAddr, CString& errorMessage)
{
	if (hostname == NULL) hostname = L"localhost";
	if (LooksNumeric(hostname)) {
		struct in_addr addr{};
		if (InetPtonW(AF_INET, hostname, &addr) != 1) {
			errorMessage = L"Invalid IP address.";
			return false;
		}
		outAddr = (int)addr.s_addr;
		return true;
	}
	return ResolveByDns(hostname, &outAddr, errorMessage);
}
