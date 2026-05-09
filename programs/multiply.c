#include <stdio.h>

int multiply(int a, int b) {
    if (b == 0) return 0;
    return a + multiply(a, b - 1);
}

int main(void) {
    int result = multiply(3, 2);
    printf("%d\n", result);
    return 0;
}
