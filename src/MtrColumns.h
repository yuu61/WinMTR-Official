#ifndef MTRCOLUMNS_H_
#define MTRCOLUMNS_H_

#define MTR_NR_COLS 9

const wchar_t MTR_COLS[ MTR_NR_COLS ][10] = {
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

const int MTR_COL_LENGTH[ MTR_NR_COLS ] = {
        190, 30, 50, 40, 40, 50, 50, 50, 50
};

#endif // MTRCOLUMNS_H_
