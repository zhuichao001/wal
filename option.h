#ifndef _WAL_OPTION_H_
#define _WAL_OPTION_H_

typedef struct{
    bool nosync;
    int segmentlimit;   //the upper limit of segment
    int cachednum;      //cached segments in memory
}option;

#endif
