#include <stdio.h>
#include <string.h>
#include "walog.h"

int readcheck(wal::walog &wal, int *idxs,  int size){
    for(int i=0; i<size; ++i){
        int idx = idxs[i];
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", idx);

        std::string data;
        if(wal.read(idx, data)<0){
            printf("read %d failed, not exist\n", idx);
            continue;
        }

        if(strcmp(tmp, data.c_str())!=0){
            printf("not match, expect:%s, data:%s\n", tmp, data.c_str());
        }
        printf("yes match, expect:%s, data:%s\n", tmp, data.c_str());
    }
    return 0;
}

int test0(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    wal.write(1, "hello, world 1");
    wal.write(2, "hello, world 2");
    wal.write(3, "hello, world 3");

    std::string s;
    wal.read(2, s);
    fprintf(stderr, "read %d:%s\n", 2, s.c_str());
    return 0;
}

int test1(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    for(int i=1; i<1000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);
        wal.write(i, tmp);
    }

    for(int i=0; i<1000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);

        std::string data;
        wal.read(i, data);

        if(strcmp(tmp, data.c_str())!=0){
            printf("not match, expect:%s, data:%s\n", tmp, data.c_str());
            break;
        }
    }
    return 0;
}

int test2(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    for(int i=1; i<1000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);
        wal.write(i, tmp);
    }

    int idxs[4] = {1, 2, 888888, 999999};
    readcheck(wal, idxs, 4);
    return 0;
}

int test3(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    for(int i=1; i<1000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);
        wal.write(i, tmp);
    }
    return 0;
}

int test4(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    for(int i=1; i<1000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);
        wal.write(i, tmp);
    }

    wal.truncateback(780838);

    int idxs[4] = {1, 780837, 780838, 780839};
    readcheck(wal, idxs, 4);
    return 0;
}

int test5(){
    wal::option defaultopt = {20971520, 3};
    wal::walog wal(defaultopt, "./data/");
    for(int i=1; i<8000000; ++i){
        char tmp[32];
        sprintf(tmp, "hello, world %d\0", i);
        wal.write(i, tmp);
    }

    wal.truncatefront(780837);

    int idxs[4] = {1, 780836, 780837, 780838};
    readcheck(wal, idxs, 4);
    return 0;
}

int main(){
    test5();
    return 0;
}
