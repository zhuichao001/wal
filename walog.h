#ifndef _WAL_LOG_H_
#define _WAL_LOG_H_

#include <string>
#include <vector>
#include <mutex>
#include "segment.h"


class walog{
    std::string directory;
    std::mutex mux;
    std::vector<segment*> segments;
    int firstidx;
    int lastidx;
    segment *seg; //tail segment
    bool isclosed;
    bool iscorrupt;
public:
    walog(std::string path):
        directory(path),
        firstidx(0),
        lastidx(-1),
        isclosed(false),
        iscorrupt(false) {
        seg = new segment(path.c_str());
    }

    int load(){
        return 0;
    }

    bool firstindex(int &index){
        std::lock_guard<std::mutex> lock(mux);
        if(isclosed || iscorrupt){
            return false;
        }
        index = firstidx;
        return true;
    }

    bool lastindex(int &index){
        std::lock_guard<std::mutex> lock(mux);
        if(isclosed || iscorrupt){
            return false;
        }
        index = lastidx;
        return true;
    }

    int append(const char *data, const int len){
        std::lock_guard<std::mutex> lock(mux);
        if(isclosed || iscorrupt){
            return -1;
        }

        return seg->append(data, len);
    }

};

#endif
