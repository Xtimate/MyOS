#include "include/fs.h"
#include "include/kstring.h"

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} __attribute__((packed)) ustar_header_t;

static void *fs_archive;
static unsigned int fs_size;

static unsigned int parse_octal(const char *s, int len) {
    unsigned int val = 0;
    for (int i = 0; i < len && s[i] >= '0' && s[i] <= '7'; i++) {
        val = val * 8 + (s[i] - '0');
    }
    return val;
}

int fs_init(void *archive, unsigned int size) {
    fs_archive = archive;
    fs_size = size;
    return 0;
}

fs_file_t fs_open(const char *name) {
    fs_file_t result = {0, 0};
    unsigned char *ptr = (unsigned char *)fs_archive;
    unsigned char *end = ptr + fs_size;

    while (ptr + 512 <= end) {
        ustar_header_t *hdr = (ustar_header_t *)ptr;

        if (hdr->name[0] == '\0') break;

        unsigned int size = parse_octal(hdr->size, 12);

        if (kstrcmp(hdr->name, name) == 0) {
            result.data = (char *)(ptr + 512);
            result.size = size;
            return result;
        }

        unsigned int blocks = (size + 511) / 512;
        ptr += 512 + blocks * 512;
    }

    return result;
}

int fs_list(char (*names)[100], int max_names) {
    int count = 0;
    unsigned char *ptr = (unsigned char *)fs_archive;
    unsigned char *end = ptr + fs_size;

    while (ptr + 512 <= end && count < max_names) {
        ustar_header_t *hdr = (ustar_header_t *)ptr;
        if (hdr->name[0] == '\0') break;

        kstrcpy(names[count], hdr->name);
        count++;

        unsigned int size = parse_octal(hdr->size, 12);
        unsigned int blocks = (size + 511) / 512;
        ptr += 512 + blocks * 512;
    }

    return count;
}
