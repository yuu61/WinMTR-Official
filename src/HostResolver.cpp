//*****************************************************************************
// FILE:            HostResolver.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "HostResolver.h"

namespace {

bool TryParseNumeric(LPCWSTR hostname, IpAddress& outAddr)
{
	in_addr v4{};
	in6_addr v6{};
	if (InetPtonW(AF_INET, hostname, &v4) == 1) {
		outAddr = IpAddress::FromIPv4(v4);
		return true;
	}
	if (InetPtonW(AF_INET6, hostname, &v6) == 1) {
		outAddr = IpAddress::FromIPv6(v6);
		return true;
	}
	return false;
}

bool ResolveByDns(LPCWSTR hostname, IpAddress* outAddr, CString& errorMessage)
{
	ADDRINFOW hints   = {};
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_RAW;
	PADDRINFOW result = nullptr;
	if (GetAddrInfoW(hostname, nullptr, &hints, &result) != 0 || result == nullptr) {
		errorMessage = L"Unable to resolve hostname.";
		return false;
	}

	bool ok = false;
	for (PADDRINFOW ai = result; ai != nullptr; ai = ai->ai_next) {
		if (ai->ai_family == AF_INET) {
			if (outAddr) {
				const auto* sa = reinterpret_cast<const sockaddr_in*>(ai->ai_addr);
				*outAddr = IpAddress::FromIPv4(sa->sin_addr);
			}
			ok = true;
			break;
		}
		if (ai->ai_family == AF_INET6) {
			if (outAddr) {
				const auto* sa = reinterpret_cast<const sockaddr_in6*>(ai->ai_addr);
				*outAddr = IpAddress::FromIPv6(sa->sin6_addr);
			}
			ok = true;
			break;
		}
	}
	FreeAddrInfoW(result);
	if (!ok) {
		errorMessage = L"Unable to resolve hostname.";
	}
	return ok;
}

} // namespace


//*****************************************************************************
// HostResolver::LooksNumeric
//
//*****************************************************************************
bool HostResolver::LooksNumeric(LPCWSTR hostname)
{
	if (hostname == nullptr) {
		return false;
	}
	IpAddress tmp;
	return TryParseNumeric(hostname, tmp);
}


//*****************************************************************************
// HostResolver::Validate
//
//*****************************************************************************
bool HostResolver::Validate(LPCWSTR hostname, CString& errorMessage)
{
	if (hostname == nullptr) {
		hostname = L"localhost";
	}
	IpAddress tmp;
	if (TryParseNumeric(hostname, tmp)) {
		return true;
	}
	return ResolveByDns(hostname, nullptr, errorMessage);
}


//*****************************************************************************
// HostResolver::Resolve
//
//*****************************************************************************
bool HostResolver::Resolve(LPCWSTR hostname, IpAddress& outAddr, CString& errorMessage)
{
	if (hostname == nullptr) {
		hostname = L"localhost";
	}
	if (TryParseNumeric(hostname, outAddr)) {
		return true;
	}
	return ResolveByDns(hostname, &outAddr, errorMessage);
}


//*****************************************************************************
// HostResolver::ReverseResolve
//
//*****************************************************************************
bool HostResolver::ReverseResolve(const IpAddress& addr, wchar_t* outName, size_t outSize)
{
	if (addr.family == AF_INET) {
		sockaddr_in sa = {};
		sa.sin_family   = AF_INET;
		sa.sin_addr     = addr.bytes.v4;
		return GetNameInfoW(reinterpret_cast<const sockaddr*>(&sa), sizeof(sa),
		                    outName, static_cast<DWORD>(outSize),
		                    nullptr, 0, NI_NAMEREQD) == 0;
	}
	if (addr.family == AF_INET6) {
		sockaddr_in6 sa = {};
		sa.sin6_family  = AF_INET6;
		sa.sin6_addr    = addr.bytes.v6;
		return GetNameInfoW(reinterpret_cast<const sockaddr*>(&sa), sizeof(sa),
		                    outName, static_cast<DWORD>(outSize),
		                    nullptr, 0, NI_NAMEREQD) == 0;
	}
	return false;
}


//*****************************************************************************
// HostResolver::FormatNumeric
//
//*****************************************************************************
void HostResolver::FormatNumeric(const IpAddress& addr, wchar_t* outName, size_t outSize)
{
	if (outSize == 0) {
		return;
	}
	outName[0] = L'\0';
	if (addr.family == AF_INET) {
		InetNtopW(AF_INET, &addr.bytes.v4, outName, outSize);
	} else if (addr.family == AF_INET6) {
		InetNtopW(AF_INET6, &addr.bytes.v6, outName, outSize);
	}
}
