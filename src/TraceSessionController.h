//*****************************************************************************
// FILE:            TraceSessionController.h
//
// DESCRIPTION:
//   State machine that sequences the trace lifecycle (IDLE/TRACING/STOPPING/
//   EXIT) and forwards UI side-effects through ISessionView.
//
//   Completion notification uses PostMessage (via ISessionView::PostTrace*)
//   instead of mutex polling; the owning dialog wires the messages back to
//   OnTraceCompleted / OnTraceFailed.
//
//*****************************************************************************

#ifndef TRACESESSIONCONTROLLER_H_
#define TRACESESSIONCONTROLLER_H_

#include "TraceOptions.h"
#include <afxwin.h>
#include <memory>
#include <string>
#include <thread>

class HopStatistics;
class ISessionView;
class TraceEngine;

class TraceSessionController {
public:
	enum class State {
		Idle,
		Tracing,
		Stopping,
		Exit
	};

	explicit TraceSessionController(ISessionView* view);
	~TraceSessionController();

	TraceSessionController(const TraceSessionController&)            = delete;
	TraceSessionController& operator=(const TraceSessionController&) = delete;

	void RequestStart(const std::wstring& host, const TraceOptions& opts);
	void RequestStop();
	void RequestExit();

	// Called on the UI thread when the session worker reports back through
	// ISessionView::PostTrace*.
	void OnTraceCompleted();
	void OnTraceFailed(const CString& error);

	// Driven by WM_TIMER; refreshes the hop list while TRACING/STOPPING.
	void Tick();

	[[nodiscard]] State CurrentState() const { return state_; }
	[[nodiscard]] bool  IsTracing()   const { return state_ == State::Tracing; }
	[[nodiscard]] const HopStatistics& Stats() const;

	[[nodiscard]] bool    IsEngineValid() const;
	[[nodiscard]] LPCWSTR EngineError()   const;

private:
	enum class Transition {
		IdleToIdle,
		IdleToTracing,
		IdleToExit,
		TracingToTracing,
		TracingToStopping,
		TracingToIdle,
		TracingToExit,
		StoppingToIdle,
		StoppingToStopping,
		StoppingToExit
	};

	void Transit(State new_state);
	void ApplyTransition();
	void RestoreIdleUi();

	void ExecuteSession(std::wstring host, TraceOptions opts);

	ISessionView*                view_;
	std::unique_ptr<TraceEngine> engine_;
	State                        state_;
	Transition                   transition_;
	unsigned int                 tick_count_;
	std::thread                  session_thread_;

	std::wstring pending_host_;
	TraceOptions pending_opts_{};
};

#endif // TRACESESSIONCONTROLLER_H_
