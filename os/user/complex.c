#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void reverse(char *s) {
    int len = strlen(s);
    for (int i = 0; i < len / 2; i++) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}

void _start() {
    // test printf with formatting
    printf("--- complex test ---\n");

    // test factorial (recursion + arithmetic)
    for (int i = 1; i <= 8; i++) {
        printf("%d! = %d\n", i, factorial(i));
    }

    // test malloc + realloc + string ops
    char *buf = malloc(16);
    strcpy(buf, "hello");
    buf = realloc(buf, 32);
    strcat(buf, " world");
    printf("string: %s\n", buf);

    // test reverse
    reverse(buf);
    printf("reversed: %s\n", buf);

    // test a dynamic array
    int n = 10;
    int *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i * i;
    printf("squares: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    free(arr);
    free(buf);

    printf("--- done ---\n");
    exit(0);
}
