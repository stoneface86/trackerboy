
#include "tbb.h"

#include <stdio.h>

int main(void) {

    B::NimMain();
    B::init();

    printf("Version: %s\n", B::version());

    B::uninit();
    return 0;
}
