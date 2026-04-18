//*****************************************************************************
// FILE:            TraceSessionController.h
//
// DESCRIPTION:
//   State machine that sequences the trace lifecycle (IDLE/TRACING/STOPPING/
//   EXIT) and forwards UI side-effects through ISessionView.
//
//*****************************************************************************

#ifndef TRACESESSIONCONTROLLER_H_
#define TRACESESSIONCONTROLLER_H_

#include "TraceOptions.h"
#include <memory>
#include <string>

class WinMTRNet;
class ISessionView;

class TraceSessionController {
public:
	enum State {
		IDLE,
		TRACING,
		STOPPING,
		EXIT
	};

	explicit TraceSessionController(ISessionView* view);
	~TraceSessionController();

	TraceSessionController(const TraceSessionController&)            = delete;
	TraceSessionController& operator=(const TraceSessionController&) = delete;

	void RequestStart(const std::wstring& host, const TraceOptions& opts);
	void RequestStop();
	void RequestExit();

	// Called from the dialog's WM_TIMER handler.
	void Tick();

	[[nodiscard]] State      CurrentState() const { return state_; }
	[[nodiscard]] bool       IsTracing()    const { return state_ == TRACING; }
	[[nodiscard]] WinMTRNet& Net();

private:
	enum Transition {
		IDLE_TO_IDLE,
		IDLE_TO_TRACING,
		IDLE_TO_EXIT,
		TRACING_TO_TRACING,
		TRACING_TO_STOPPING,
		TRACING_TO_EXIT,
		STOPPING_TO_IDLE,
		STOPPING_TO_STOPPING,
		STOPPING_TO_EXIT
	};

	void Transit(State new_state);

	ISessionView*              view_;
	std::unique_ptr<WinMTRNet> net_;
	HANDLE                     mutex_;
	State                      state_;
	Transition                 transition_;
	unsigned int               tick_count_;

	std::wstring               pending_host_;
	TraceOptions               pending_opts_{};
};

#endif // TRACESESSIONCONTROLLER_H_
