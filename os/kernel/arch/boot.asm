KERNEL_VIRTUAL_BASE equ 0xC0000000

section .multiboot2
align 8
mb2_start:
    dd 0xE85250D6                          ; multiboot2 magic
    dd 0                                   ; architecture: i386
    dd mb2_end - mb2_start                 ; header length
    dd -(0xE85250D6 + 0 + (mb2_end - mb2_start)) ; checksum

    ; framebuffer tag
    align 8
    dw 5                                   ; type: framebuffer
    dw 0                                   ; flags
    dd 20                                  ; size
    dd 1920                                ; width
    dd 1080                                ; height
    dd 32                                  ; depth

    ; end tag
    align 8
    dw 0
    dw 0
    dd 8
mb2_end:

section .boot
bits 32

align 4096
boot_page_directory:
    times 1024 dd 0

align 4096
boot_page_table:
    times 1024 dd 0

align 4096
boot_page_table2:
    times 1024 dd 0

global start
extern kernel_main

start:
    mov ebp, ebx                           ; save mbi pointer in ebp
    mov edi, boot_page_table
    mov esi, 0
    mov ecx, 1024
.fill_table:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    add esi, 0x1000
    add edi, 4
    loop .fill_table

    mov edi, boot_page_table2
    mov esi, 0x400000
    mov ecx, 1024
.fill_table2:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    add esi, 0x1000
    add edi, 4
    loop .fill_table2

    mov eax, boot_page_table
    or eax, 0x3
    mov [boot_page_directory], eax
    mov [boot_page_directory + 768*4], eax

    mov eax, boot_page_table2
    or eax, 0x3
    mov [boot_page_directory + 4], eax
    mov [boot_page_directory + 769*4], eax

    mov ecx, boot_page_directory
    mov cr3, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp ecx

section .text
bits 32
higher_half:
    mov dword [boot_page_directory], 0
    mov dword [boot_page_directory + 4], 0
    invlpg [0]

    mov esp, stack_top

    push 0
    popf
    push ebp      ; mbi pointer
    push eax
    call kernel_main
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
