//*****************************************************************************
// FILE:            Global.h
//
// DESCRIPTION:
//   Common MFC / CRT / Winsock includes shared by every translation unit.
//
//*****************************************************************************

#ifndef GLOBAL_H_
#define GLOBAL_H_

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#define VC_EXTRALEAN

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif
#include <afxsock.h>
#include <ws2tcpip.h>

#include <process.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <format>

#include "resource.h"

#endif // GLOBAL_H_
