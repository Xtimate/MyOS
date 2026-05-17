global jump_usermode
jump_usermode:
    mov ecx, [esp + 4]
    mov edx, [esp + 8]

    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    push 0x23
    push edx
    pushf
    pop eax
    or eax, 0x200
    and eax, 0xFFFFBFFF
    push eax
    push 0x1B
    push ecx
    iret
