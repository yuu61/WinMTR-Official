//*****************************************************************************
// FILE:            StartTraceUseCase.cpp
//*****************************************************************************

#include "Global.h"
#include "StartTraceUseCase.h"
#include "HostResolver.h"

namespace StartTraceUseCase {

Result Validate(const CString& rawHost)
{
	Result r;
	r.normalizedHost = rawHost;
	r.normalizedHost.TrimLeft();
	r.normalizedHost.TrimRight();

	if (r.normalizedHost.IsEmpty()) {
		r.validation = Validation::EmptyHost;
		return r;
	}

	if (!WinMTRHostResolver::Validate(r.normalizedHost, r.error)) {
		r.validation = Validation::ResolutionFailed;
		return r;
	}

	r.validation = Validation::Ok;
	return r;
}

} // namespace StartTraceUseCase
