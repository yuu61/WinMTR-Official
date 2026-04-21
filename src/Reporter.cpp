//*****************************************************************************
// FILE:            Reporter.cpp
//
//
//*****************************************************************************

#include "Global.h"
#include "Reporter.h"
#include "HopStatistics.h"

#include <fstream>
#include <vector>

namespace {
constexpr size_t kHostnameBufferLen = 255;
constexpr size_t kUtf8BomSize       = 3;
} // namespace


//*****************************************************************************
// Reporter::BuildTextReport
//
//*****************************************************************************
CString Reporter::BuildTextReport(const HopStatistics& stats)
{
	wchar_t buf[kHostnameBufferLen];
	CString result;

	const int nh = stats.GetMax();

	result = L"|------------------------------------------------------------------------------------------|\r\n";
	result += L"|                                      WinMTR statistics                                   |\r\n";
	result += L"|                       Host              -   %  | Sent | Recv | Best | Avrg | Wrst | Last |\r\n";
	result += L"|------------------------------------------------|------|------|------|------|------|------|\r\n";

	for (int i = 0; i < nh; i++) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') {
			wcscpy_s(buf, L"No response from host");
		}

		CString line;
		line.Format(L"|%40s - %4d | %4d | %4d | %4d | %4d | %4d | %4d |\r\n",
		            buf, stats.GetPercent(i),
		            stats.GetXmit(i), stats.GetReturned(i), stats.GetBest(i),
		            stats.GetAvg(i), stats.GetWorst(i), stats.GetLast(i));
		result += line;
	}

	result += L"|________________________________________________|______|______|______|______|______|______|\r\n";
	result += L"   ";
	result += CString(MAKEINTRESOURCE(IDS_STRING_SB_NAME));

	return result;
}


//*****************************************************************************
// Reporter::BuildHtmlReport
//
//*****************************************************************************
CString Reporter::BuildHtmlReport(const HopStatistics& stats)
{
	wchar_t buf[kHostnameBufferLen];
	CString result;

	const int nh = stats.GetMax();

	result = L"<html><head><title>WinMTR Statistics</title></head><body bgcolor=\"white\">\r\n";
	result += L"<center><h2>WinMTR statistics</h2></center>\r\n";
	result += L"<p align=\"center\"> <table border=\"1\" align=\"center\">\r\n";
	result += L"<tr><td>Host</td> <td>%</td> <td>Sent</td> <td>Recv</td> <td>Best</td> <td>Avrg</td> <td>Wrst</td> <td>Last</td></tr>\r\n";

	for (int i = 0; i < nh; i++) {
		stats.GetName(i, buf, _countof(buf));
		if (buf[0] == L'\0') {
			wcscpy_s(buf, L"No response from host");
		}

		CString line;
		line.Format(L"<tr><td>%s</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td> <td>%4d</td></tr>\r\n",
		            buf, stats.GetPercent(i),
		            stats.GetXmit(i), stats.GetReturned(i), stats.GetBest(i),
		            stats.GetAvg(i), stats.GetWorst(i), stats.GetLast(i));
		result += line;
	}

	result += L"</table></body></html>\r\n";

	return result;
}


//*****************************************************************************
// Reporter::CopyToClipboard
//
//*****************************************************************************
bool Reporter::CopyToClipboard(CWnd* owner, const CString& content)
{
	if (owner == nullptr) {
		return false;
	}
	if (!owner->OpenClipboard()) {
		return false;
	}
	EmptyClipboard();

	const SIZE_T bytes      = (static_cast<SIZE_T>(content.GetLength()) + 1) * sizeof(wchar_t);
	HGLOBAL      clipbuffer = GlobalAlloc(GMEM_DDESHARE, bytes);
	if (clipbuffer == nullptr) {
		CloseClipboard();
		return false;
	}
	auto* buffer = static_cast<wchar_t*>(GlobalLock(clipbuffer));
	if (buffer == nullptr) {
		GlobalFree(clipbuffer);
		CloseClipboard();
		return false;
	}
	const SIZE_T chars = bytes / sizeof(wchar_t);
	wcscpy_s(buffer, chars, static_cast<LPCWSTR>(content));
	GlobalUnlock(clipbuffer);

	HANDLE h = SetClipboardData(CF_UNICODETEXT, clipbuffer);
	if (!h) {
		GlobalFree(clipbuffer);
	}
	CloseClipboard();
	return h != nullptr;
}


//*****************************************************************************
// Reporter::SaveToFile
//
//*****************************************************************************
bool Reporter::SaveToFile(LPCWSTR path, const CString& content)
{
	// MSVC extends std::ofstream with a wide-path constructor, so we can
	// hand the Unicode path directly instead of going through _wfopen.
	// std::ios::binary and std::ios::trunc are unsigned bitmask values so the
	// bitwise OR is well-defined despite the hicpp-signed-bitwise check.
	std::ofstream out(path, std::ios::binary | std::ios::trunc); // NOLINT(hicpp-signed-bitwise)
	if (!out) {
		return false;
	}

	// UTF-8 BOM so the resulting file is unambiguously UTF-8 for downstream
	// tools (Notepad, editors without an encoding picker).
	constexpr char kUtf8Bom[] = "\xEF\xBB\xBF";
	out.write(kUtf8Bom, kUtf8BomSize);

	const int wideLen = content.GetLength();
	if (wideLen > 0) {
		const int utf8Len = WideCharToMultiByte(CP_UTF8, 0,
		                                        static_cast<LPCWSTR>(content), wideLen,
		                                        nullptr, 0, nullptr, nullptr);
		if (utf8Len <= 0) {
			return false;
		}

		std::vector<char> utf8(static_cast<size_t>(utf8Len));
		WideCharToMultiByte(CP_UTF8, 0,
		                    static_cast<LPCWSTR>(content), wideLen,
		                    utf8.data(), utf8Len, nullptr, nullptr);
		out.write(utf8.data(), utf8Len);
	}

	out.flush();
	out.close();
	return out.good();
}
