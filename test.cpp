#include <stdio.h>
#include <string.h>
#include "walog.h"

int test0(){
    option defaultopt = {20971520, 3};
    walog wal(&defaultopt, "./data/");
    wal.write(1, "hello, world 1");
    wal.write(2, "hello, world 2");
    wal.write(3, "hello, world 3");

    std::string s;
    wal.read(2, s);
    fprintf(stderr, "read %d:%s\n", 2, s.c_str());
    return 0;
}

int main(){
    test0();
    return 0;
}
