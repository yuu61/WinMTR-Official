//*****************************************************************************
// FILE:            Main.h
//
//
// DESCRIPTION:
//
//
// NOTES:
//
//
//*****************************************************************************

#ifndef WINMTRMAIN_H_
#define WINMTRMAIN_H_

#include "Dialog.h"


//*****************************************************************************
// CLASS:  WinMTRMain
//
//
//*****************************************************************************

class WinMTRMain : public CWinApp
{
public:
	WinMTRMain();

	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#endif // ifndef WINMTRMAIN_H_
