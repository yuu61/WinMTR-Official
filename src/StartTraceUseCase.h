//*****************************************************************************
// FILE:            StartTraceUseCase.h
//
// DESCRIPTION:
//   Pure validation step for the "start trace" action: trims the raw host
//   string, rejects empty input, and verifies DNS resolvability. The dialog
//   handles any UI side-effects based on the returned Result.
//
//*****************************************************************************

#ifndef STARTTRACEUSECASE_H_
#define STARTTRACEUSECASE_H_

#include <afxwin.h>

namespace StartTraceUseCase {

enum class Validation {
	Ok,
	EmptyHost,
	ResolutionFailed,
};

struct Result {
	Validation validation = Validation::Ok;
	CString normalizedHost;
	CString error;
};

// Trims rawHost, rejects empty, then synchronously probes DNS. Returns a
// Result the caller can branch on. No UI calls.
[[nodiscard]] Result Validate(const CString& rawHost);

} // namespace StartTraceUseCase

#endif // STARTTRACEUSECASE_H_
