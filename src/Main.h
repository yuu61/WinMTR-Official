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

#ifndef MAIN_H_
#define MAIN_H_

#include "Dialog.h"


//*****************************************************************************
// CLASS:  Main
//
//
//*****************************************************************************

class Main : public CWinApp {
public:
	Main() = default;

	BOOL InitInstance() override;

	DECLARE_MESSAGE_MAP()
};

#endif // ifndef MAIN_H_
