#include <stdio.h>
#include <string.h>
#include "walog.h"

int main(){
    walog wal("./001.dat");
    const char *s = "hello, world.";
    wal.append(s, strlen(s));
    return 0;
}
