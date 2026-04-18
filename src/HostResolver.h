//*****************************************************************************
// FILE:            HostResolver.h
//
//
// DESCRIPTION:
//   IPv4 hostname/address resolution helpers.
//
//*****************************************************************************

#ifndef WINMTRHOSTRESOLVER_H_
#define WINMTRHOSTRESOLVER_H_

#include <afxwin.h>

//*****************************************************************************
// CLASS:  WinMTRHostResolver
//
//
//*****************************************************************************

class WinMTRHostResolver
{
public:
	static bool LooksNumeric(LPCSTR hostname);

	// Validates that hostname can be resolved. For numeric IP strings, skips
	// the DNS round-trip (legacy InitMTRNet behavior).
	static bool Validate(LPCSTR hostname, CString& errorMessage);

	// Resolves hostname to an IPv4 address in network byte order.
	// Numeric strings go through inet_pton; otherwise getaddrinfo.
	static bool Resolve(LPCSTR hostname, int& outAddr, CString& errorMessage);
};

#endif // ifndef WINMTRHOSTRESOLVER_H_
