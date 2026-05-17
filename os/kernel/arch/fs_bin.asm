section .rodata
global fs_archive_start
global fs_archive_end

fs_archive_start:
    incbin "fs.tar"
fs_archive_end:
