//*****************************************************************************
// FILE:            IcmpIO.h
//
// DESCRIPTION:
//   Per-worker RAII wrapper around the static-linked iphlpapi ICMP APIs.
//   Owns the IPv4/IPv6 ICMP handles and the async completion event used by
//   IcmpSendEcho2 / Icmp6SendEcho2. Uses IcmpParseReplies to decode the
//   asynchronous reply buffer.
//
//*****************************************************************************

#ifndef ICMPIO_H_
#define ICMPIO_H_

#include <afxwin.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include "IpAddress.h"

constexpr DWORD ECHO_REPLY_TIMEOUT = 5000;

// Reply buffer must hold at least one ICMP_ECHO_REPLY (or ICMPV6_ECHO_REPLY)
// plus the echoed request data plus 8 bytes of headroom, per MSDN. For the
// asynchronous IcmpSendEcho2 path the driver prepends an IO_STATUS_BLOCK-
// sized header before the reply record, so reserve extra slack.
constexpr size_t ICMP_REPLY_HEADROOM = 64;

class IcmpIO {
public:
	IcmpIO();
	~IcmpIO();
	IcmpIO(const IcmpIO&)            = delete;
	IcmpIO& operator=(const IcmpIO&) = delete;

	[[nodiscard]] bool    IsValid()   const { return valid_; }
	[[nodiscard]] LPCWSTR LastError() const { return last_error_; }

	// Issues an asynchronous echo, waits for either completion or stop_event,
	// and parses the reply. Returns the number of replies (0 on timeout,
	// failure, or stop). On success the caller reads reply_buffer as
	// ICMP_ECHO_REPLY (AF_INET) or ICMPV6_ECHO_REPLY (AF_INET6).
	DWORD DoEcho(const IpAddress& dest, UCHAR ttl,
	             LPVOID req_data, WORD req_size,
	             LPVOID reply_buffer, DWORD reply_size,
	             HANDLE stop_event, DWORD timeout);

private:
	bool    valid_;
	HANDLE  v4_handle_;
	HANDLE  v6_handle_;
	HANDLE  echo_event_;
	LPCWSTR last_error_;
};

#endif // ICMPIO_H_
