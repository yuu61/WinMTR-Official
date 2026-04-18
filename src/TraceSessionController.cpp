//*****************************************************************************
// FILE:            TraceSessionController.cpp
//*****************************************************************************

#include "Global.h"
#include "TraceSessionController.h"
#include "SessionView.h"
#include "TraceEngine.h"
#include "HostResolver.h"
#include "IpAddress.h"

#include <sstream>
#include <utility>

#define TRACE_MSG(msg)                                       \
	{                                                        \
		std::wostringstream dbg(std::wostringstream::out);   \
		dbg << msg << std::endl;                             \
		OutputDebugStringW(dbg.str().c_str());               \
	}

TraceSessionController::TraceSessionController(ISessionView* view)
	: view_(view),
	  engine_(std::make_unique<TraceEngine>()),
	  state_(IDLE),
	  transition_(IDLE_TO_IDLE),
	  tick_count_(0)
{
}

TraceSessionController::~TraceSessionController()
{
	// The session worker always posts back via PostTrace*, which drives the
	// state machine to IDLE / EXIT and the thread exits shortly after. Wait
	// for it here so no worker thread outlives the controller.
	if (session_thread_.joinable()) session_thread_.join();
}

const HopStatistics& TraceSessionController::Stats() const
{
	return engine_->Stats();
}

bool TraceSessionController::IsEngineValid() const
{
	return engine_ && engine_->IsValid();
}

LPCWSTR TraceSessionController::EngineError() const
{
	return engine_ ? engine_->LastError() : nullptr;
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
	if ((tick_count_ % 10 == 0) && (state_ == TRACING || state_ == STOPPING)) {
		view_->RefreshList();
	}
}

void TraceSessionController::OnTraceCompleted()
{
	// Previous session worker has posted completion on the UI thread, so it
	// is safe to join and reset for the next session.
	if (session_thread_.joinable()) session_thread_.join();

	switch (state_) {
	case TRACING:
	case STOPPING:
		Transit(IDLE);
		break;
	case EXIT:
		view_->RequestClose();
		break;
	case IDLE:
		// Not expected; benign.
		break;
	}
}

void TraceSessionController::OnTraceFailed(const CString& error)
{
	if (session_thread_.joinable()) session_thread_.join();

	switch (state_) {
	case TRACING:
	case STOPPING:
		view_->ShowError(error);
		Transit(IDLE);
		break;
	case EXIT:
		view_->RequestClose();
		break;
	case IDLE:
		break;
	}
}

void TraceSessionController::Transit(State new_state)
{
	switch (new_state) {
	case IDLE:
		switch (state_) {
		case STOPPING: transition_ = STOPPING_TO_IDLE; break;
		case TRACING:  transition_ = TRACING_TO_IDLE;  break;
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
		return;
	}

	ApplyTransition();
}

void TraceSessionController::ApplyTransition()
{
	switch (transition_) {
	case IDLE_TO_TRACING: {
		view_->SetStartEnabled(false);
		view_->SetStartText(L"Stop");
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		view_->SetStatus(L"Double click on host name for more information.");

		// Defensive: a stale completion message might still be queued, so
		// ensure any previous worker is joined before overwriting the thread.
		if (session_thread_.joinable()) session_thread_.join();

		try {
			session_thread_ = std::thread(&TraceSessionController::ExecuteSession,
			                              this, pending_host_, pending_opts_);
		} catch (const std::system_error&) {
			view_->ShowError(L"Failed to start trace worker.");
			Transit(IDLE);
			return;
		}

		view_->SetStartEnabled(true);
		break;
	}
	case IDLE_TO_IDLE:
		break;
	case STOPPING_TO_IDLE:
	case TRACING_TO_IDLE:
		RestoreIdleUi();
		break;
	case STOPPING_TO_STOPPING:
	case TRACING_TO_TRACING:
		view_->RefreshList();
		break;
	case TRACING_TO_STOPPING:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		engine_->Stop();
		view_->SetStatus(L"Waiting for last packets in order to stop trace ...");
		view_->RefreshList();
		break;
	case IDLE_TO_EXIT:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		// No worker to wait on; close immediately.
		view_->RequestClose();
		break;
	case TRACING_TO_EXIT:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		engine_->Stop();
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

void TraceSessionController::RestoreIdleUi()
{
	view_->SetStartEnabled(true);
	view_->SetStatus(CString((LPCWSTR)IDS_STRING_SB_NAME));
	view_->SetStartText(L"Start");
	view_->SetHostComboEnabled(true);
	view_->SetOptionsEnabled(true);
	view_->FocusHostCombo();
}

void TraceSessionController::ExecuteSession(std::wstring host, TraceOptions opts)
{
	IpAddress addr;
	CString   err;
	if (!HostResolver::Resolve(host.c_str(), addr, err)) {
		view_->PostTraceFailed(err);
		return;
	}

	engine_->Trace(addr, opts);
	view_->PostTraceCompleted();
}
