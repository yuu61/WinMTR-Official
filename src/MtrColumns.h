#ifndef MTRCOLUMNS_H_
#define MTRCOLUMNS_H_

#include <array>

constexpr int MTR_NR_COLS = 9;

constexpr std::array<const wchar_t*, MTR_NR_COLS> MTR_COLS = {
        L"Hostname",
        L"Nr",
        L"Loss %",
        L"Sent",
        L"Recv",
        L"Best",
        L"Avrg",
        L"Worst",
        L"Last"
};

// Column widths in pixels at 96 DPI, sized for Segoe UI 9pt.
// Hostname gets the most space; numeric columns are sized for 3-4 digit values.
constexpr std::array<int, MTR_NR_COLS> MTR_COL_LENGTH = {
        220, 36, 52, 48, 48, 52, 52, 56, 52
};

// Right-align numeric columns so digits line up at the decimal place.
constexpr std::array<int, MTR_NR_COLS> MTR_COL_FORMAT = {
        LVCFMT_LEFT,   // Hostname
        LVCFMT_RIGHT,  // Nr
        LVCFMT_RIGHT,  // Loss %
        LVCFMT_RIGHT,  // Sent
        LVCFMT_RIGHT,  // Recv
        LVCFMT_RIGHT,  // Best
        LVCFMT_RIGHT,  // Avrg
        LVCFMT_RIGHT,  // Worst
        LVCFMT_RIGHT,  // Last
};

#endif // MTRCOLUMNS_H_
