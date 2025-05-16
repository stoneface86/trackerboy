
#include "tbb.hxx"

#include <cstdio>

int main(void) {

    B::NimMain();
    B::init();

    printf("Version: %s\n", B::version());

    B::deinit();
    return 0;
}
