//*****************************************************************************
// FILE:            IcmpIO.h
//
// DESCRIPTION:
//   RAII wrapper around ICMP.DLL. Owns the dynamic library, the function
//   pointers, and the IcmpCreateFile handle.
//
//*****************************************************************************

#ifndef ICMPIO_H_
#define ICMPIO_H_

#include <afxwin.h>
#include <ws2tcpip.h>

typedef ip_option_information IPINFO, *PIPINFO, FAR *LPIPINFO;

#ifdef _WIN64
typedef icmp_echo_reply32 ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#else
typedef icmp_echo_reply ICMPECHO, *PICMPECHO, FAR *LPICMPECHO;
#endif

constexpr DWORD ECHO_REPLY_TIMEOUT = 5000;

class IcmpIO {
public:
	IcmpIO();
	~IcmpIO();
	IcmpIO(const IcmpIO&)            = delete;
	IcmpIO& operator=(const IcmpIO&) = delete;

	[[nodiscard]] bool    IsValid()   const { return valid_; }
	[[nodiscard]] LPCWSTR LastError() const { return last_error_; }

	// Matches IcmpSendEcho semantics; returns 0 on failure or if invalid.
	DWORD SendEcho(u_long dest_addr, LPVOID req_data, WORD req_size,
	               LPIPINFO ip_info, LPVOID reply_buffer, DWORD reply_size,
	               DWORD timeout);

private:
	typedef HANDLE (WINAPI *LPFNICMPCREATEFILE)(VOID);
	typedef BOOL   (WINAPI *LPFNICMPCLOSEHANDLE)(HANDLE);
	typedef DWORD  (WINAPI *LPFNICMPSENDECHO)(HANDLE, u_long, LPVOID, WORD,
	                                         LPVOID, LPVOID, DWORD, DWORD);

	bool                valid_;
	HINSTANCE           dll_;
	HANDLE              handle_;
	LPFNICMPCREATEFILE  fn_create_;
	LPFNICMPCLOSEHANDLE fn_close_;
	LPFNICMPSENDECHO    fn_send_;
	LPCWSTR             last_error_;
};

#endif // ICMPIO_H_
