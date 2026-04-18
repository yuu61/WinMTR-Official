//*****************************************************************************
// FILE:            TraceSessionController.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceSessionController.h"
#include "SessionView.h"
#include "Net.h"
#include <sstream>

#define TRACE_MSG(msg)                                      \
	{                                                       \
		std::wostringstream dbg(std::wostringstream::out);  \
		dbg << msg << std::endl;                            \
		OutputDebugStringW(dbg.str().c_str());              \
	}

TraceSessionController::TraceSessionController(ISessionView* view)
	: view_(view),
	  net_(std::make_unique<WinMTRNet>()),
	  mutex_(CreateMutex(NULL, FALSE, NULL)),
	  state_(IDLE),
	  transition_(IDLE_TO_IDLE),
	  tick_count_(0)
{
}

TraceSessionController::~TraceSessionController()
{
	CloseHandle(mutex_);
}

WinMTRNet& TraceSessionController::Net()
{
	return *net_;
}

void TraceSessionController::RequestStart(const std::wstring& host, const TraceOptions& opts)
{
	pending_host_ = host;
	pending_opts_ = opts;
	Transit(TRACING);
}

void TraceSessionController::RequestStop()
{
	Transit(STOPPING);
}

void TraceSessionController::RequestExit()
{
	Transit(EXIT);
}

void TraceSessionController::Tick()
{
	tick_count_ += 1;

	if (state_ == EXIT && WaitForSingleObject(mutex_, 0) == WAIT_OBJECT_0) {
		ReleaseMutex(mutex_);
		view_->RequestClose();
	}

	if (WaitForSingleObject(mutex_, 0) == WAIT_OBJECT_0) {
		ReleaseMutex(mutex_);
		Transit(IDLE);
	} else if ((tick_count_ % 10 == 0) && (WaitForSingleObject(mutex_, 0) == WAIT_TIMEOUT)) {
		ReleaseMutex(mutex_);
		if      (state_ == TRACING)  Transit(TRACING);
		else if (state_ == STOPPING) Transit(STOPPING);
	}
}

void TraceSessionController::Transit(State new_state)
{
	switch (new_state) {
	case IDLE:
		switch (state_) {
		case STOPPING: transition_ = STOPPING_TO_IDLE; break;
		case IDLE:     transition_ = IDLE_TO_IDLE;     break;
		default:
			TRACE_MSG(L"Received state IDLE after " << state_);
			return;
		}
		state_ = IDLE;
		break;
	case TRACING:
		switch (state_) {
		case IDLE:    transition_ = IDLE_TO_TRACING;    break;
		case TRACING: transition_ = TRACING_TO_TRACING; break;
		default:
			TRACE_MSG(L"Received state TRACING after " << state_);
			return;
		}
		state_ = TRACING;
		break;
	case STOPPING:
		switch (state_) {
		case STOPPING: transition_ = STOPPING_TO_STOPPING; break;
		case TRACING:  transition_ = TRACING_TO_STOPPING;  break;
		default:
			TRACE_MSG(L"Received state STOPPING after " << state_);
			return;
		}
		state_ = STOPPING;
		break;
	case EXIT:
		switch (state_) {
		case IDLE:     transition_ = IDLE_TO_EXIT;     break;
		case STOPPING: transition_ = STOPPING_TO_EXIT; break;
		case TRACING:  transition_ = TRACING_TO_EXIT;  break;
		case EXIT: break;
		default:
			TRACE_MSG(L"Received state EXIT after " << state_);
			return;
		}
		state_ = EXIT;
		break;
	default:
		TRACE_MSG(L"Received state " << state_);
	}

	switch (transition_) {
	case IDLE_TO_TRACING:
		view_->SetStartEnabled(false);
		view_->SetStartText(L"Stop");
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		view_->SetStatus(L"Double click on host name for more information.");
		net_->BeginTraceAsync(pending_host_, pending_opts_, mutex_);
		view_->SetStartEnabled(true);
		break;
	case IDLE_TO_IDLE:
		break;
	case STOPPING_TO_IDLE:
		view_->SetStartEnabled(true);
		view_->SetStatus(CString((LPCWSTR)IDS_STRING_SB_NAME));
		view_->SetStartText(L"Start");
		view_->SetHostComboEnabled(true);
		view_->SetOptionsEnabled(true);
		view_->FocusHostCombo();
		break;
	case STOPPING_TO_STOPPING:
		view_->RefreshList();
		break;
	case TRACING_TO_TRACING:
		view_->RefreshList();
		break;
	case TRACING_TO_STOPPING:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		net_->StopTrace();
		view_->SetStatus(L"Waiting for last packets in order to stop trace ...");
		view_->RefreshList();
		break;
	case IDLE_TO_EXIT:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		break;
	case TRACING_TO_EXIT:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		net_->StopTrace();
		view_->SetStatus(L"Waiting for last packets in order to stop trace ...");
		break;
	case STOPPING_TO_EXIT:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		break;
	default:
		TRACE_MSG(L"Unknown transition " << transition_);
	}
}
