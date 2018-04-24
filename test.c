// test

#include <stdio.h>
#include <stdlib.h>

static void test(int *x) {
    while (*x % 4 != 0) {
        (*x)++;
    }
}

void main(void) {
    int x = 5001;
    test(&x);
    printf("%d\n", x);
}
