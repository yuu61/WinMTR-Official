//*****************************************************************************
// FILE:            Main.cpp
//
//
// HISTORY:
//
//
//    -- versions 0.8
//
// - 01.18.2002 - Store LRU hosts in registry (v0.8)
// - 05.08.2001 - Replace edit box with combo box which hold last entered hostnames.
//				  Fixed a memory leak which caused program to crash after a long
//				  time running. (v0.7)
// - 11.27.2000 - Added resizing support and flat buttons. (v0.6)
// - 11.26.2000 - Added copy data to clipboard and posibility to save data to file as text or HTML.(v0.5)
// - 08.03.2000 - added double-click on hostname for detailed information (v0.4)
// - 08.02.2000 - fix icmp error codes handling. (v0.3)
// - 08.01.2000 - support for full command-line parameter specification (v0.2)
// - 07.30.2000 - support for command-line host specification
//					by Silviu Simen (ssimen@ubisoft.ro) (v0.1b)
// - 07.28.2000 - first release (v0.1)
//*****************************************************************************

#include "Global.h"
#include "Main.h"
#include "Dialog.h"
#include "Help.h"
#include "CommandLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

WinMTRMain WinMTR;

//*****************************************************************************
// BEGIN_MESSAGE_MAP
//
//
//*****************************************************************************
BEGIN_MESSAGE_MAP(WinMTRMain, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

//*****************************************************************************
// WinMTRMain::WinMTRMain
//
//
//*****************************************************************************
WinMTRMain::WinMTRMain()
{
}

//*****************************************************************************
// WinMTRMain::InitInstance
//
//
//*****************************************************************************
BOOL WinMTRMain::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	WinMTRDialog mtrDialog;
	m_pMainWnd = &mtrDialog;

	if (wcslen(m_lpCmdLine)) {
		wcscat(m_lpCmdLine, L" ");
		auto parsed = WinMTRCommandLine::Parse(m_lpCmdLine);
		if (parsed.helpRequested) {
			WinMTRHelp mtrHelp;
			m_pMainWnd = &mtrHelp;
			mtrHelp.DoModal();
			return FALSE;
		}
		if (parsed.hostName) {
			mtrDialog.SetHostName(parsed.hostName->c_str());
		}
		mtrDialog.SetCommandLineOverrides(parsed.overrides);
	}

	mtrDialog.DoModal();

	return FALSE;
}
