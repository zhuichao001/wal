#ifndef _WAL_LOG_H_
#define _WAL_LOG_H_

#include <string>
#include <vector>
#include <mutex>
#include "fio.h"
#include "util.h"
#include "segment.h"


class walog{
    std::mutex mux;
    std::vector<segment*> segments;
    int firstidx;
    int lastidx;
    segment *seg; //current/tail segment
    bool isclosed;
    bool iscorrupt;
public:
    walog(const option *opt):
        firstidx(1),
        lastidx(0),
        seg(nullptr),
        isclosed(false),
        iscorrupt(false) {
    }

    int open(const char *path){
        mkdir(path);
        std::vector<std::string> files;
        ls(path, files);
        if(files.size()>0){
            return recover(files);
        }

        char compath[256];
        sprintf(compath, "%s/%09d\0", path, 1);
        seg = new segment(1, compath);
        segments.push_back(seg);
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

private:
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

        for(auto it= start>=0?files.begin()+start:files.begin(); it < (end>=0?files.begin()+end:files.end()); ++it){
            const std::string &path = (*it);
            int index = atoi(filename(path.c_str()));
            segments.push_back(new segment(index, path.c_str()));
        }
        seg = *(segments.end()-1);

        return 0;
    }

};

#endif
