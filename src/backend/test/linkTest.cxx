
#include "tbb.hxx"

#include <cstdio>

int main(void) {

    bNimMain();
    bInit();

    printf("Version: %s\n", bVersion());

    bDeinit();
    return 0;
}
