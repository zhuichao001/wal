#ifndef _WALOG_UTIL_H_
#define _WALOG_UTIL_H_

#include <string>

bool startswith(const std::string &s, const std::string &sub){
    return s.find(sub)==0 ? true : false;
}

bool endswith(const std::string &s, const std::string &sub){
    return s.rfind(sub)==(s.length()-sub.length()) ? true : false;
}

const char* filename(const char *path) {
    const char *p = strrchr(path,'/');
    return p ? p+1 : path;
}

void pack(int from , char *to){
    for(int i=0; i<sizeof(int); ++i){
        to[i] = from & 0xff;
        from >>=8;
    }
}

#endif
