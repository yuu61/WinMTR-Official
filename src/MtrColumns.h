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

constexpr std::array<int, MTR_NR_COLS> MTR_COL_LENGTH = {
        190, 30, 50, 40, 40, 50, 50, 50, 50
};

#endif // MTRCOLUMNS_H_
