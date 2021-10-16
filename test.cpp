#include <stdio.h>
#include <string.h>
#include "walog.h"

int main(){
    option defaultopt = {false, 20971520, 3};
    walog wal(&defaultopt);
    wal.open("./data/");
    const char *s = "hello, world.";
    //wal.append(s, strlen(s));
    return 0;
}
