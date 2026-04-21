//*****************************************************************************
// FILE:            LRUStore.h
//
// DESCRIPTION:
//   Persists the MRU host list in HKCU\Software\WinMTR\LRU. Owns both the
//   in-memory count and the maximum capacity. Max is mirrored in the Config
//   registry key (written by Settings), but behaviourally belongs here.
//
//*****************************************************************************

#ifndef LRUSTORE_H_
#define LRUSTORE_H_

#include "TraceConfig.h"
#include <afxwin.h>
#include <vector>

class LRUStore {
public:
	void SetMax(int max) { max_ = max; }
	[[nodiscard]] int Max() const { return max_; }
	[[nodiscard]] int Count() const { return nr_; }

	// Reads NrLRU + Host{1..Max()} from the registry. Populates outHosts and
	// initializes Count(). Returns false on registry failure.
	[[nodiscard]] bool Load(std::vector<CString>& outHosts);

	// Stores host at the next slot; wraps Count() back to 1 when it reaches Max().
	void Append(LPCWSTR host);

	// Deletes Host{Max()+1}..Host{Count()} and caps Count() at Max().
	void Trim();

	// Deletes every stored host and resets Count() to zero.
	void Clear();

private:
	int nr_ = 0;
	int max_ = DEFAULT_MAX_LRU;
};

#endif // LRUSTORE_H_
