//*****************************************************************************
// FILE:            IcmpIO.cpp
//*****************************************************************************

#include "Global.h"
#include "IcmpIO.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

IcmpIO::IcmpIO()
	: valid_(false),
	  v4_handle_(INVALID_HANDLE_VALUE),
	  v6_handle_(INVALID_HANDLE_VALUE),
	  echo_event_(NULL),
	  last_error_(nullptr)
{
	v4_handle_ = IcmpCreateFile();
	v6_handle_ = Icmp6CreateFile();
	if (v4_handle_ == INVALID_HANDLE_VALUE && v6_handle_ == INVALID_HANDLE_VALUE) {
		last_error_ = L"IcmpCreateFile and Icmp6CreateFile both failed.";
		return;
	}

	echo_event_ = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (echo_event_ == NULL) {
		last_error_ = L"CreateEvent failed.";
		return;
	}

	valid_ = true;
}

IcmpIO::~IcmpIO()
{
	// Close ICMP handles first: per MSDN, IcmpCloseHandle cancels any pending
	// I/O issued against the handle, so the echo event cannot be signaled
	// after it is closed.
	if (v4_handle_ != INVALID_HANDLE_VALUE) IcmpCloseHandle(v4_handle_);
	if (v6_handle_ != INVALID_HANDLE_VALUE) IcmpCloseHandle(v6_handle_);
	if (echo_event_ != NULL)                CloseHandle(echo_event_);
}

DWORD IcmpIO::DoEcho(const IpAddress& dest, UCHAR ttl,
                     LPVOID req_data, WORD req_size,
                     LPVOID reply_buffer, DWORD reply_size,
                     HANDLE stop_event, DWORD timeout)
{
	if (!valid_) return 0;

	IP_OPTION_INFORMATION ip_opts = {};
	ip_opts.Ttl   = ttl;
	ip_opts.Flags = 0x02; // IP_FLAG_DF

	DWORD io_result = 0;

	if (dest.family == AF_INET) {
		if (v4_handle_ == INVALID_HANDLE_VALUE) return 0;
		io_result = IcmpSendEcho2(
			v4_handle_, echo_event_, NULL, NULL,
			dest.bytes.v4.s_addr,
			req_data, req_size, &ip_opts,
			reply_buffer, reply_size, timeout);
	} else if (dest.family == AF_INET6) {
		if (v6_handle_ == INVALID_HANDLE_VALUE) return 0;

		sockaddr_in6 src = {};
		src.sin6_family = AF_INET6;
		sockaddr_in6 dst = {};
		dst.sin6_family = AF_INET6;
		dst.sin6_addr   = dest.bytes.v6;

		io_result = Icmp6SendEcho2(
			v6_handle_, echo_event_, NULL, NULL,
			&src, &dst,
			req_data, req_size, &ip_opts,
			reply_buffer, reply_size, timeout);
	} else {
		return 0;
	}

	// Async: expect 0 + ERROR_IO_PENDING. Anything else is a failure path.
	if (io_result != 0 || GetLastError() != ERROR_IO_PENDING)
		return 0;

	const HANDLE waits[2] = { echo_event_, stop_event };
	const DWORD  n_waits  = (stop_event != NULL) ? 2 : 1;
	// Add a small slack so the driver has time to post the reply before our
	// wait times out, matching the synchronous IcmpSendEcho behavior.
	const DWORD  wait_ms  = timeout + 1000;

	const DWORD wr = WaitForMultipleObjects(n_waits, waits, FALSE, wait_ms);
	if (wr != WAIT_OBJECT_0) {
		// Stop signaled, timeout, or error: the pending I/O will be cancelled
		// when the ICMP handle is closed in ~IcmpIO.
		return 0;
	}

	if (dest.family == AF_INET6) {
		return Icmp6ParseReplies(reply_buffer, reply_size);
	}
	return IcmpParseReplies(reply_buffer, reply_size);
}
