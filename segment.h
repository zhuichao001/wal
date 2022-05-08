#ifndef _WAL_SEGMENT_H_
#define _WAL_SEGMENT_H_

#include <string>
#include "fio.h"
#include "memfile.h"

namespace wal{

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
        if(fio::fexist(fpath)){
            pmem.load(fpath);
        } else {
            pmem.create(fpath);
        }
    }

    segment(int idx, const char *fpath, const char *data, int len):
        index(idx),
        path(fpath){
        pmem.create(fpath);
        pmem.restore(data, len);
    }

    int write(int idx, const char *data, int len){
        /*
        if(idx!=pmem.lastindex()+1){
            fprintf(stderr, "error! idx:%d is not equal to (lastindex:%d)+1\n", idx, pmem.lastindex());
            return -1;
        }
        */
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

    int leftsize(){
        return (MEM_FILE_LIMIT - sizeof(int)*2 - pmem.size());
    }

    int release(){
        ::remove(path.c_str());
        return 0;
    }

    segment *clonehalf(const int CLONE_HALF, int idx, const char *newpath){
        segment *seg = nullptr;
        char *data = nullptr;
        int len =0;
        if(CLONE_HALF==FIRST_HALF){ //first half
            pmem.firsthalf(idx, &data, &len);
            seg = new segment(index, newpath, data, len);
        }else{ //second half
            pmem.secondhalf(idx, &data, &len);
            seg = new segment(idx, newpath, data, len);
        }
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

    int endindex(){
        return pmem.lastindex();
    }
};

} //end of namepsace wal

#endif
