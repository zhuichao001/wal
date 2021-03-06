#ifndef _WAL_LOG_H_
#define _WAL_LOG_H_

#include <string>
#include <vector>
#include <algorithm>
#include <mutex>
#include "fio.h"
#include "util.h"
#include "segment.h"

namespace wal{

typedef struct{
    int segmentlimit;   //the upper limit of segment
    int cachednum;      //cached segments in memory
}option;

class walog{
    std::mutex mux;
    std::vector<segment*> segments;
    const option opt;
    const std::string dirpath;
    int firstidx;
    int lastidx;
    segment *seg; //current/tail segment
    bool isclosed;
    bool iscorrupt;
public:
    walog(const option p, const char *path):
        opt(p),
        dirpath(path),
        firstidx(1),
        lastidx(0),
        seg(nullptr),
        isclosed(false),
        iscorrupt(false) {
        open();
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

    int write(int index, const std::string &data){
        std::lock_guard<std::mutex> lock(mux);
        if(isclosed || iscorrupt){
            return -1;
        }

        if(index != lastidx+1){
            fprintf(stderr, "logindex:%d not match lastidx:%d.\n", index, lastidx);
            return -1;
        }

        if(seg->leftsize() < int(data.size())){
            cycle();
        }

        seg->write(index, data.c_str(), data.size());
        lastidx = index;

        return 0;
    }

    int read(int index, std::string &data){
        std::lock_guard<std::mutex> lock(mux);
        if(isclosed || iscorrupt){
            return -1;
        }

        segment * dst = find(index);
        if(dst==nullptr){
            return -1;
        }

        char *body=nullptr;
        int len=0;
        if(dst->read(index, &body, &len)<0){
            return -1;
        }
        data.resize(len);
        data.assign(body, len);
        return 0;
    }

    int truncatefront(int index){
        std::lock_guard<std::mutex> lock(mux);
        if(index <=0 || lastidx==0 || index<firstidx || index>lastidx){
            return -1; //out of range
        }
        if(index==firstidx){
            return 0;
        }

        segment *dst = find(index);
        if(dst==nullptr){
            return -1;
        }

        char tmppath[256];
        sprintf(tmppath, "%s/%09d.START", dirpath.c_str(), dst->startindex());
        segment *neo = dst->clonehalf(SECOND_HALF, index, tmppath);

        auto it = segments.begin();
        do{
            segment *cur = *it;
            (*it)->release();
            delete *it;
            segments.erase(it);
            if(cur==dst){
                break;
            }
        }while(true);

        char newpath[256];
        sprintf(newpath, "%s/%09d", dirpath.c_str(), index);
        neo->repath(newpath);

        segments.insert(segments.begin(), neo);
        firstidx = index;
        return 0;
    }

    int truncateback(int index){
        std::lock_guard<std::mutex> lock(mux);
        if(index <=0 || lastidx==0 || index<firstidx || index>lastidx+1){
            return -1; //out of range
        }
        if(index==lastidx+1){
            return 0;
        }

        segment *dst = find(index);
        if(dst==nullptr){
            return -1;
        }

        int startidx = dst->startindex();

        char tmppath[256];
        sprintf(tmppath, "%s/%09d.END", dirpath.c_str(), startidx);
        segment *neo = dst->clonehalf(FIRST_HALF, index, tmppath);

        auto startit = std::find(segments.begin(), segments.end(), dst);
        for(auto it = startit; it!=segments.end();){
            (*it)->release();
            delete *it;
            segments.erase(it);
        }

        char newpath[256];
        sprintf(newpath, "%s/%09d", dirpath.c_str(), startidx);
        neo->repath(newpath);

        segments.insert(segments.end(), neo);
        lastidx = index-1;
        return 0;
    }

private:
    int open(){
        fio::mkdir(dirpath.c_str());
        std::vector<std::string> files;
        fio::flist(dirpath.c_str(), files);
        if(files.size()>0){
            return recover(files);
        }else{
            return cycle(); //create a new segment
        }
    }

    // Cycle the old segment for a new segment.
    int cycle(){
        int nextidx = lastidx+1;
        char path[256];
        sprintf(path, "%s/%09d", dirpath.c_str(), nextidx);
        seg = new segment(nextidx, path);
        segments.push_back(seg);
        return 0;
    }

    segment *find(int index){
        int i = 0;
        int j = segments.size();
        while(i < j){
            int m = i + (j-i)/2;
            if(index >= segments[m]->startindex()){
                i = m + 1;
            }else{
                j = m;
            }
        }
        if(i==0){
            return nullptr;
        }
        return i==0 ? nullptr : segments[i-1];
    }

    int recover(const std::vector<std::string> &files){
        int start = -1;
        int end = -1;
        std::vector<std::string> rubbish;
        for(auto it = files.begin(); it!=files.end(); ++it){
            const std::string &path = (*it);
            if(endswith(path, ".START")){
                start = segments.size();
                rubbish.insert(rubbish.end(), files.begin(), it);
            }else if(endswith(path, ".END")){
                end = segments.size();
                rubbish.insert(rubbish.end(), it+1, files.end());
                break;
            }
        }

        for(auto it = rubbish.begin(); it!=rubbish.end(); ++it){
            ::remove((*it).c_str());
        }

        if(start>=0){
            std::string newpath(files[start].c_str(), files[start].length()-strlen(".START"));
            ::rename(files[start].c_str(), newpath.c_str());
        }

        if(end>=0){
            std::string newpath(files[end].c_str(), files[end].length()-strlen(".END"));
            ::rename(files[end].c_str(), newpath.c_str());
        }

        for(auto it= start>=0?files.begin()+start:files.begin(); it < (end>=0?files.begin()+end+1:files.end()); ++it){
            const std::string &path = (*it);
            int index = atoi(filename(path.c_str()));
            segments.push_back(new segment(index, path.c_str()));
        }
        seg = *(segments.end()-1);
        firstidx = segments[0]->startindex();
        lastidx = seg->endindex();

        return 0;
    }

};

} //end of namespace wal

#endif
