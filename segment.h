#ifndef _WAL_SEGMENT_H_
#define _WRITE_AHEAD_LOG_H_

#include <string>
#include "option.h"
#include "fio.h"
#include "memfile.h"

class segment{
    int index;
    memfile pmem;
    static option opt;
public:
    segment(int idx, const char *path):
        index(idx){
        if(fexist(path)){
            pmem.load(path);
        } else {
            pmem.create(path);
        }
    }

    int append(const char *data, const int len){
        return pmem.write(data, len);
    }

    int full(){
        if(pmem.size() >= opt.segmentlimit){
            return true;
        }
        return false;
    }
};

#endif
