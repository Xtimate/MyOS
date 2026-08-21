#ifndef KSTRING_H
#define KSTRING_H

int kstrlen(const char *str);
char *kstrcpy(char *dest, const char *src);
int kstrcmp(const char *a, const char *b);
char *kstrcat(char *dest, const char *src);
int kstrncmp(const char *a, const char *b, int n);
char *kstrchr(const char *str, char c);
void *kmemset(void *ptr, int value, unsigned int size);
void *kmemcpy(void *dest, const void *src, unsigned int size);

#endif
