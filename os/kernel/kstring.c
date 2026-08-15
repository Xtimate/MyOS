#include "include/kstring.h"

int kstrlen(const char *str) {
    int i = 0;
    while (str[i]) i++;
    return i;
}

char *kstrcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

int kstrcmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return a[i] - b[i];
        i++;
    }
    return a[i] - b[i];
}

char *kstrcat(char *dest, const char *src) {
    int i = kstrlen(dest);
    int j = 0;
    while (src[j]) {
        dest[i++] = src[j++];
    }
    dest[i] = 0;
    return dest;
}

char *kstrchr(const char *str, char c) {
    int i = 0;
    while (str[i]) {
        if (str[i] == c) return (char *)&str[i];
        i++;
    }
    return 0;
}

void *kmemset(void *ptr, int value, unsigned int size) {
    unsigned char *p = (unsigned char *)ptr;
    for (unsigned int i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

void *kmemcpy(void *dest, const void *src, unsigned int size) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (unsigned int i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return dest;
}
