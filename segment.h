#ifndef _WAL_SEGMENT_H_
#define _WRITE_AHEAD_LOG_H_

#include <string>
#include "option.h"
#include "fio.h"
#include "memfile.h"

const int FIRST_HALF = 0;
const int SECOND_HALF = 1;

class segment{
    int index; //start index
    std::string path;
    memfile pmem;
public:
    segment(int idx, const char *fpath):
        index(idx),
        path(fpath){
        if(fexist(fpath)){
            pmem.load(fpath);
            index = pmem.firstindex();
        } else {
            pmem.create(fpath);
        }
    }

    int write(int idx, const char *data, int len){
        if(index<0){
            index = idx;
        }
        return pmem.write(idx, data, len);
    }

    int read(int idx, char **data, int *len){
        return pmem.read(idx, data, len);
    }

    int restore(const char *data, int len){
        return pmem.restore(data, len);
    }

    int size(){
        return pmem.size();
    }

    int release(){
        ::remove(path.c_str());
        return 0;
    }

    segment *clonehalf(const int CLONE_HALF, int index, const char *newpath){
        char *data = nullptr;
        int len =0;
        if(CLONE_HALF==FIRST_HALF){ //first half
            pmem.firsthalf(index, &data, &len);
        }else{ //second half
            pmem.secondhalf(index, &data, &len);
        }
        segment *seg = new segment(index, newpath);
        seg->restore(data, len);
        return seg;
    }

    int repath(const char *newpath){
        ::rename(path.c_str(), newpath);
        path = newpath;
        return 0;
    }

    int startindex(){
        return index;
    }
};

#endif
