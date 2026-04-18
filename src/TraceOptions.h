//*****************************************************************************
// FILE:            TraceOptions.h
//
//
// DESCRIPTION:
//   Snapshot of trace-time configuration passed from Dialog into Net.
//
//*****************************************************************************

#ifndef TRACEOPTIONS_H_
#define TRACEOPTIONS_H_

#include <windef.h>

// Values are captured at DoTrace() entry. Changes made via the Options
// dialog during an active trace do not affect the in-flight trace.
struct TraceOptions {
	int    pingsize;
	double interval;
	BOOL   useDNS;
};

#endif // TRACEOPTIONS_H_
