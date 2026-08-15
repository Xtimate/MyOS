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

KERNEL_VIRTUAL_BASE equ 0xC0000000
section .boot
bits 32

align 32
boot_pdpt: times 4 dq 0            ; 4 entries, 1GB each, covers full 4GB

align 4096
boot_pd_low:  times 512 dq 0       ; PD for PDPT[0]  (identity, 0-1GB)
align 4096
boot_pd_high: times 512 dq 0       ; PD for PDPT[3]  (higher-half, 3-4GB)

align 4096
boot_pt0: times 512 dq 0           ; covers phys 0x000000-0x1FFFFF (2MB)
align 4096
boot_pt1: times 512 dq 0           ; covers phys 0x200000-0x3FFFFF
align 4096
boot_pt2: times 512 dq 0           ; covers phys 0x400000-0x5FFFFF
align 4096
boot_pt3: times 512 dq 0           ; covers phys 0x600000-0x7FFFFF

global start
extern kernel_main
start:
    mov ebp, ebx                   ; save mbi pointer (unchanged from before)

    ; fill 4 page tables, each mapping 512 x 4KB pages (2MB), identity
    mov edi, boot_pt0
    mov esi, 0x000000
    mov ecx, 512
.fill_pt0:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    mov dword [edi+4], 0
    add esi, 0x1000
    add edi, 8
    loop .fill_pt0

    mov edi, boot_pt1
    mov esi, 0x200000
    mov ecx, 512
.fill_pt1:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    mov dword [edi+4], 0
    add esi, 0x1000
    add edi, 8
    loop .fill_pt1

    mov edi, boot_pt2
    mov esi, 0x400000
    mov ecx, 512
.fill_pt2:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    mov dword [edi+4], 0
    add esi, 0x1000
    add edi, 8
    loop .fill_pt2

    mov edi, boot_pt3
    mov esi, 0x600000
    mov ecx, 512
.fill_pt3:
    mov eax, esi
    or eax, 0x3
    mov [edi], eax
    mov dword [edi+4], 0
    add esi, 0x1000
    add edi, 8
    loop .fill_pt3

    ; wire the 4 PTs into both PD_low (identity) and PD_high (higher-half)
    mov eax, boot_pt0
    or eax, 0x3
    mov [boot_pd_low],    eax
    mov dword [boot_pd_low+4], 0
    mov [boot_pd_high],   eax
    mov dword [boot_pd_high+4], 0

    mov eax, boot_pt1
    or eax, 0x3
    mov [boot_pd_low+8],  eax
    mov dword [boot_pd_low+12], 0
    mov [boot_pd_high+8], eax
    mov dword [boot_pd_high+12], 0

    mov eax, boot_pt2
    or eax, 0x3
    mov [boot_pd_low+16], eax
    mov dword [boot_pd_low+20], 0
    mov [boot_pd_high+16], eax
    mov dword [boot_pd_high+20], 0

    mov eax, boot_pt3
    or eax, 0x3
    mov [boot_pd_low+24], eax
    mov dword [boot_pd_low+28], 0
    mov [boot_pd_high+24], eax
    mov dword [boot_pd_high+28], 0

    ; PDPT[0] -> PD_low (identity, covers virt 0-1GB)
    mov eax, boot_pd_low
    or eax, 0x1
    mov [boot_pdpt], eax
    mov dword [boot_pdpt+4], 0

    ; PDPT[3] -> PD_high (covers virt 0xC0000000-0xFFFFFFFF)
    mov eax, boot_pd_high
    or eax, 0x1
    mov [boot_pdpt + 24], eax
    mov dword [boot_pdpt + 28], 0

    mov eax, boot_pdpt
    mov cr3, eax

    mov eax, cr4
    or eax, 0x20                   ; CR4.PAE (bit 5)
    mov cr4, eax

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp ecx

section .text
bits 32
higher_half:
    mov dword [boot_pd_low], 0
    mov dword [boot_pd_low+4], 0
    mov dword [boot_pd_low+8], 0
    mov dword [boot_pd_low+12], 0
    mov dword [boot_pd_low+16], 0
    mov dword [boot_pd_low+20], 0
    mov dword [boot_pd_low+24], 0
    mov dword [boot_pd_low+28], 0
    invlpg [0]
    invlpg [0x200000]
    invlpg [0x400000]
    invlpg [0x600000]

    mov esp, stack_top
    extern bss_start
    extern bss_end
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb
    push 0
    popf
    push ebp
    push eax
    call kernel_main
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
