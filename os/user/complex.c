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

int main(int argc, char **argv) {
    printf("--- complex test ---\n");
    for (int i = 1; i <= 8; i++) {
        printf("%d! = %d\n", i, factorial(i));
    }
    char *buf = malloc(16);
    strcpy(buf, "hello");
    buf = realloc(buf, 32);
    strcat(buf, " world");
    printf("string: %s\n", buf);
    reverse(buf);
    printf("reversed: %s\n", buf);
    int n = 10;
    int *arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) arr[i] = i * i;
    printf("squares: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
    free(arr);
    free(buf);
    printf("--- done ---\n");
    return 0;
}
