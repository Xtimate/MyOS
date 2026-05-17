global switch_context
global process_start

switch_context:
    mov edx, [esp + 4]
    mov ecx, [esp + 8]
    mov eax, [esp + 12]

    mov [edx], esp
    mov esp, ecx
    mov cr3, eax

    ret

process_start:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    mov esp, eax
    mov cr3, ebx
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
