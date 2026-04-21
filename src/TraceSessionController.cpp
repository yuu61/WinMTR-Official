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

// Variadic so the expression chain (operator<< list) can be passed through
// verbatim; parenthesising a stream chain the way bugprone-macro-parentheses
// wants would break the left-most `wchar_t*` operand.
#define TRACE_MSG(...)                                     \
	do {                                                   \
		std::wostringstream dbg(std::wostringstream::out); \
		dbg << __VA_ARGS__ << std::endl;                   \
		OutputDebugStringW(dbg.str().c_str());             \
	} while (false)

namespace {
constexpr unsigned int kRefreshTicksDivisor = 10;
} // namespace

TraceSessionController::TraceSessionController(ISessionView* view)
    : view_(view),
      engine_(std::make_unique<TraceEngine>())
{
}

TraceSessionController::~TraceSessionController()
{
	// The session worker always posts back via PostTrace*, which drives the
	// state machine to IDLE / EXIT and the thread exits shortly after. Wait
	// for it here so no worker thread outlives the controller.
	if (session_thread_.joinable()) {
		session_thread_.join();
	}
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
	Transit(State::Tracing);
}

void TraceSessionController::RequestStop()
{
	Transit(State::Stopping);
}

void TraceSessionController::RequestExit()
{
	Transit(State::Exit);
}

void TraceSessionController::Tick()
{
	tick_count_ += 1;
	if ((tick_count_ % kRefreshTicksDivisor == 0) && (state_ == State::Tracing || state_ == State::Stopping)) {
		view_->RefreshList();
	}
}

void TraceSessionController::OnTraceCompleted()
{
	// Previous session worker has posted completion on the UI thread, so it
	// is safe to join and reset for the next session.
	if (session_thread_.joinable()) {
		session_thread_.join();
	}

	switch (state_) {
	case State::Tracing:
	case State::Stopping:
		Transit(State::Idle);
		break;
	case State::Exit:
		view_->RequestClose();
		break;
	case State::Idle:
		// Not expected; benign.
		break;
	}
}

void TraceSessionController::OnTraceFailed(const CString& error)
{
	if (session_thread_.joinable()) {
		session_thread_.join();
	}

	switch (state_) {
	case State::Tracing:
	case State::Stopping:
		view_->ShowError(error);
		Transit(State::Idle);
		break;
	case State::Exit:
		view_->RequestClose();
		break;
	case State::Idle:
		break;
	}
}

void TraceSessionController::Transit(State new_state)
{
	switch (new_state) {
	case State::Idle:
		switch (state_) {
		case State::Stopping: transition_ = Transition::StoppingToIdle; break;
		case State::Tracing:  transition_ = Transition::TracingToIdle;  break;
		case State::Idle:     transition_ = Transition::IdleToIdle;     break;
		default:
			TRACE_MSG(L"Received state IDLE after " << static_cast<int>(state_));
			return;
		}
		state_ = State::Idle;
		break;
	case State::Tracing:
		switch (state_) {
		case State::Idle:    transition_ = Transition::IdleToTracing;    break;
		case State::Tracing: transition_ = Transition::TracingToTracing; break;
		default:
			TRACE_MSG(L"Received state TRACING after " << static_cast<int>(state_));
			return;
		}
		state_ = State::Tracing;
		break;
	case State::Stopping:
		switch (state_) {
		case State::Stopping: transition_ = Transition::StoppingToStopping; break;
		case State::Tracing:  transition_ = Transition::TracingToStopping;  break;
		default:
			TRACE_MSG(L"Received state STOPPING after " << static_cast<int>(state_));
			return;
		}
		state_ = State::Stopping;
		break;
	case State::Exit:
		switch (state_) {
		case State::Idle:     transition_ = Transition::IdleToExit;     break;
		case State::Stopping: transition_ = Transition::StoppingToExit; break;
		case State::Tracing:  transition_ = Transition::TracingToExit;  break;
		case State::Exit: break;
		default:
			TRACE_MSG(L"Received state EXIT after " << static_cast<int>(state_));
			return;
		}
		state_ = State::Exit;
		break;
	default:
		TRACE_MSG(L"Received state " << static_cast<int>(state_));
		return;
	}

	ApplyTransition();
}

void TraceSessionController::ApplyTransition()
{
	switch (transition_) {
	case Transition::IdleToTracing: {
		view_->SetStartEnabled(false);
		view_->SetStartText(L"Stop");
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		view_->SetStatus(L"Double click on host name for more information.");

		// Defensive: a stale completion message might still be queued, so
		// ensure any previous worker is joined before overwriting the thread.
		if (session_thread_.joinable()) {
			session_thread_.join();
		}

		try {
			session_thread_ = std::thread(&TraceSessionController::ExecuteSession,
			                              this, pending_host_, pending_opts_);
		} catch (const std::system_error&) {
			view_->ShowError(L"Failed to start trace worker.");
			Transit(State::Idle);
			return;
		}

		view_->SetStartEnabled(true);
		break;
	}
	case Transition::IdleToIdle:
		break;
	case Transition::StoppingToIdle:
	case Transition::TracingToIdle:
		RestoreIdleUi();
		break;
	case Transition::StoppingToStopping:
	case Transition::TracingToTracing:
		view_->RefreshList();
		break;
	case Transition::TracingToStopping:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		engine_->Stop();
		view_->SetStatus(L"Waiting for last packets in order to stop trace ...");
		view_->RefreshList();
		break;
	case Transition::IdleToExit:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		// No worker to wait on; close immediately.
		view_->RequestClose();
		break;
	case Transition::TracingToExit:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		engine_->Stop();
		view_->SetStatus(L"Waiting for last packets in order to stop trace ...");
		break;
	case Transition::StoppingToExit:
		view_->SetStartEnabled(false);
		view_->SetHostComboEnabled(false);
		view_->SetOptionsEnabled(false);
		break;
	default:
		TRACE_MSG(L"Unknown transition " << static_cast<int>(transition_));
	}
}

void TraceSessionController::RestoreIdleUi()
{
	view_->SetStartEnabled(true);
	view_->SetStatus(CString(MAKEINTRESOURCE(IDS_STRING_SB_NAME)));
	view_->SetStartText(L"Start");
	view_->SetHostComboEnabled(true);
	view_->SetOptionsEnabled(true);
	view_->FocusHostCombo();
}

void TraceSessionController::ExecuteSession(const std::wstring& host, TraceOptions opts)
{
	try {
		IpAddress addr;
		CString   err;
		if (!HostResolver::Resolve(host.c_str(), addr, err)) {
			view_->PostTraceFailed(err);
			return;
		}

		engine_->Trace(addr, opts);
		view_->PostTraceCompleted();
	} catch (const std::exception& e) {
		CString msg;
		msg.Format(L"Unexpected error in trace session: %hs", e.what());
		if (view_) {
			view_->PostTraceFailed(msg);
		}
	} catch (...) {
		if (view_) {
			view_->PostTraceFailed(L"Unexpected error in trace session.");
		}
	}
}
