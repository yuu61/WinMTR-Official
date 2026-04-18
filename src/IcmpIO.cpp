//*****************************************************************************
// FILE:            IcmpIO.cpp
//*****************************************************************************

#include "Global.h"
#include "IcmpIO.h"

IcmpIO::IcmpIO()
	: valid_(false),
	  dll_(nullptr),
	  handle_(INVALID_HANDLE_VALUE),
	  fn_create_(nullptr),
	  fn_close_(nullptr),
	  fn_send_(nullptr)
{
	dll_ = LoadLibraryW(L"ICMP.DLL");
	if (dll_ == nullptr) {
		AfxMessageBox(L"Failed: Unable to locate ICMP.DLL!");
		return;
	}

	fn_create_ = (LPFNICMPCREATEFILE)GetProcAddress(dll_, "IcmpCreateFile");
	fn_close_  = (LPFNICMPCLOSEHANDLE)GetProcAddress(dll_, "IcmpCloseHandle");
	fn_send_   = (LPFNICMPSENDECHO)GetProcAddress(dll_, "IcmpSendEcho");
	if (fn_create_ == nullptr || fn_close_ == nullptr || fn_send_ == nullptr) {
		AfxMessageBox(L"Wrong ICMP.DLL system library !");
		return;
	}

	handle_ = fn_create_();
	if (handle_ == INVALID_HANDLE_VALUE) {
		AfxMessageBox(L"Error in ICMP.DLL !");
		return;
	}

	valid_ = true;
}

IcmpIO::~IcmpIO()
{
	if (handle_ != INVALID_HANDLE_VALUE && fn_close_ != nullptr)
		fn_close_(handle_);
	if (dll_ != nullptr)
		FreeLibrary(dll_);
}

DWORD IcmpIO::SendEcho(u_long dest_addr, LPVOID req_data, WORD req_size,
                       LPIPINFO ip_info, LPVOID reply_buffer, DWORD reply_size,
                       DWORD timeout)
{
	if (!valid_) return 0;
	return fn_send_(handle_, dest_addr, req_data, req_size, ip_info,
	                reply_buffer, reply_size, timeout);
}
