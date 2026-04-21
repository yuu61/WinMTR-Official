//*****************************************************************************
// FILE:            HostResolver.h
//
//
// DESCRIPTION:
//   Hostname/address resolution helpers. Dual-stack (IPv4 + IPv6).
//
//*****************************************************************************

#ifndef HOSTRESOLVER_H_
#define HOSTRESOLVER_H_

#include <afxwin.h>

#include "IpAddress.h"

//*****************************************************************************
// CLASS:  HostResolver
//
//
//*****************************************************************************

class HostResolver {
public:
	// True if hostname is a valid numeric IPv4 or IPv6 literal.
	[[nodiscard]] static bool LooksNumeric(LPCWSTR hostname);

	// Validates that hostname can be resolved (numeric IP strings skip the
	// DNS round-trip).
	[[nodiscard]] static bool Validate(LPCWSTR hostname, CString& errorMessage);

	// Resolves hostname to an IpAddress. Accepts both IPv4 and IPv6 forms.
	[[nodiscard]] static bool Resolve(LPCWSTR hostname, IpAddress& outAddr, CString& errorMessage);

	// Reverse-resolves an IpAddress to a hostname. Writes into outName on
	// success.
	[[nodiscard]] static bool ReverseResolve(const IpAddress& addr, wchar_t* outName, size_t outSize);

	// Formats an IpAddress as its numeric string (InetNtopW).
	static void FormatNumeric(const IpAddress& addr, wchar_t* outName, size_t outSize);
};

#endif // ifndef HOSTRESOLVER_H_
