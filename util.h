#ifndef _WALOG_UTIL_H_
#define _WALOG_UTIL_H_

#include <string>

inline bool startswith(const std::string &s, const std::string &sub){
    return s.find(sub)==0 ? true : false;
}

inline bool endswith(const std::string &s, const std::string &sub){
    return s.rfind(sub)==(s.length()-sub.length()) ? true : false;
}

inline const char* filename(const char *path) {
    const char *p = strrchr(path,'/');
    return p ? p+1 : path;
}

#endif
