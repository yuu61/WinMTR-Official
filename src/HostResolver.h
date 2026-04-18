//*****************************************************************************
// FILE:            HostResolver.h
//
//
// DESCRIPTION:
//   IPv4 hostname/address resolution helpers.
//
//*****************************************************************************

#ifndef HOSTRESOLVER_H_
#define HOSTRESOLVER_H_

#include <afxwin.h>

//*****************************************************************************
// CLASS:  HostResolver
//
//
//*****************************************************************************

class HostResolver
{
public:
	[[nodiscard]] static bool LooksNumeric(LPCWSTR hostname);

	// Validates that hostname can be resolved. For numeric IP strings, skips
	// the DNS round-trip (legacy InitMTRNet behavior).
	[[nodiscard]] static bool Validate(LPCWSTR hostname, CString& errorMessage);

	// Resolves hostname to an IPv4 address in network byte order.
	// Numeric strings go through InetPtonW; otherwise GetAddrInfoW.
	[[nodiscard]] static bool Resolve(LPCWSTR hostname, int& outAddr, CString& errorMessage);

	// Reverse-resolves an IPv4 address (host byte order, matching
	// HopStatistics::GetAddr) to a hostname. Writes into outName on success.
	[[nodiscard]] static bool ReverseResolve(int addr, wchar_t* outName, size_t outSize);
};

#endif // ifndef HOSTRESOLVER_H_
