#include <string.h>

int foo(int *dest, int* src) {
    memcpy ( dest, src, 2 );

    return *dest;
}
