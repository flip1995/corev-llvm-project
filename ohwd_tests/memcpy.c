#include <string.h>

int foo(int *dest, int* src) {
    memcpy ( dest, src, 64 );

    return *dest;
}
