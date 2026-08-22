#ifndef FS_H
#define FS_H

typedef struct {
    char *data;
    unsigned int size;
} fs_file_t;

int fs_init(void *archive, unsigned int size);
fs_file_t fs_open(const char *name);
int fs_list(char names[][100], int max_names);
int fs_create(const char *name);
int fs_write(const char *name, const char *data, unsigned int len, int append);

#endif
