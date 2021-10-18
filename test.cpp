#include <stdio.h>
#include <string.h>
#include "walog.h"

int main(){
    option defaultopt = {false, 20971520, 3};
    walog wal(&defaultopt, "./data/");
    std::string s = "hello, world.";
    wal.write(1, s);
    return 0;
}
