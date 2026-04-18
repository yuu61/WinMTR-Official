#ifndef MTRCOLUMNS_H_
#define MTRCOLUMNS_H_

#define MTR_NR_COLS 9

const char MTR_COLS[ MTR_NR_COLS ][10] = {
        "Hostname",
        "Nr",
        "Loss %",
        "Sent",
        "Recv",
        "Best",
        "Avrg",
        "Worst",
        "Last"
};

const int MTR_COL_LENGTH[ MTR_NR_COLS ] = {
        190, 30, 50, 40, 40, 50, 50, 50, 50
};

#endif // MTRCOLUMNS_H_
