//*****************************************************************************
// FILE:            IpAddress.h
//
// DESCRIPTION:
//   Address-family-agnostic IP wrapper (IPv4 / IPv6). Stored in network byte
//   order so Winsock ICMP APIs can consume it directly.
//
//*****************************************************************************

#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <ws2tcpip.h>
#include <cstring>

struct IpAddress {
	ADDRESS_FAMILY family = AF_UNSPEC;
	union {
		in_addr  v4;
		in6_addr v6;
	} bytes{};

	[[nodiscard]] bool IsUnspecified() const
	{
		if (family == AF_INET)  return bytes.v4.s_addr == 0;
		if (family == AF_INET6) {
			for (int i = 0; i < 16; ++i)
				if (bytes.v6.u.Byte[i] != 0) return false;
			return true;
		}
		return true;
	}

	[[nodiscard]] bool Equals(const IpAddress& o) const
	{
		if (family != o.family) return false;
		if (family == AF_INET)  return bytes.v4.s_addr == o.bytes.v4.s_addr;
		if (family == AF_INET6) return memcmp(&bytes.v6, &o.bytes.v6, sizeof(in6_addr)) == 0;
		return true;
	}

	static IpAddress FromIPv4(in_addr v4)
	{
		IpAddress a;
		a.family   = AF_INET;
		a.bytes.v4 = v4;
		return a;
	}

	static IpAddress FromIPv6(const in6_addr& v6)
	{
		IpAddress a;
		a.family   = AF_INET6;
		a.bytes.v6 = v6;
		return a;
	}
};

#endif // IPADDRESS_H_
